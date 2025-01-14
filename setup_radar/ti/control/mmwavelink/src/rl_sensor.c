/****************************************************************************************
 * FileName     : rl_sensor.c
 *
 * Description  : This file defines the functions to configure RF/Sensor in mmwave radar device.
 *
 ****************************************************************************************
 * (C) Copyright 2014, Texas Instruments Incorporated. - TI web address www.ti.com
 *---------------------------------------------------------------------------------------
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/*
****************************************************************************************
* Revision History   :
*---------------------------------------------------------------------------------------
* Version  Date        Author             Defect No               Description
*---------------------------------------------------------------------------------------
* 0.1.0    12May2015   Kaushal Kukkar    -               Initial Version
*
* 0.5.2    23Sep2016   Kaushal Kukkar    AUTORADAR-540   Advance Frame APIs
*
* 0.6.0    15Nov2016   Kaushal Kukkar    MMWSDK-206      Dynamic Power Save API
*                      Kaushal Kukkar    AUTORADAR-571   Cascade Feature Support
*
* 0.7.0    11May2017   Jitendra Gupta    MMWSDK-450      Calibration Config/Report APIs
*                      Jitendra Gupta    MMWSDK-322      PS/PA/IF Loopback APIs
*
* 0.8.6    24Jul2017   Jitendra Gupta    MMWL-30         RF/Analog Monitoring APIs
*                      Kaushal Kukkar    MMWL-23         Big Endian Support
*
* 0.9.1       -        Jitendra Gupta    MMWL-5          Code size optimization
****************************************************************************************
*/
/******************************************************************************
 * INCLUDE FILES
 ******************************************************************************
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ti/control/mmwavelink/mmwavelink.h>
#include <ti/control/mmwavelink/include/rl_driver.h>
#include <ti/control/mmwavelink/include/rl_messages.h>
#include <ti/control/mmwavelink/include/rl_controller.h>
#include <ti/control/mmwavelink/include/rl_sensor.h>
#include <ti/control/mmwavelink/include/rl_trace.h>

/******************************************************************************
 * GLOBAL VARIABLES/DATA-TYPES DEFINITIONS
 ******************************************************************************
 */

/******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************
 */

/** @fn rlReturnVal_t rlSetChannelConfig(rlUInt8_t deviceMap, rlChanCfg_t* data)
*
*   @brief Sets the Rx and Tx Channel Configuration
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - Container for Channel Configuration data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function allows configuration of mmWave Front end for how many Receiver and
*   Transmit channels need to be enabled. It also defines whether to to enable
*   single mmWave device or multiple mmWave devices to realize a larger antenna array
*   (multiple is applicable only in AWR2243). This is applicable for given power cycle.

*   @note This is global configuration for transmit channels. Later one can chose
*   which transmit channel to be used for each chirp using Chirp configuaration API.
*   For e.g - If Chirp 0, uses TX0 and TX1, and Chirp 1 uses TX1 and TX2, One need
*   to enable TX0, TX1 and TX2 in this API. Based on the configuration, mmWave Front
*   would do necessary calibration before the transmit channel is used to transmit
*   chirps
*   @ref rlSetChirpConfig
*/
/* DesignId : MMWL_DesignId_009 */
/* Requirements : AUTORADAR_REQ-752, AUTORADAR_REQ-757, AUTORADAR_REQ-970 */
rlReturnVal_t rlSetChannelConfig(rlUInt8_t deviceMap, rlChanCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_CHAN_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlChanCfg_t));
    }

    RL_LOGV_ARG0("rlSetChannelConfig ends...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlSetAdcOutConfig(rlUInt8_t deviceMap, rlAdcOutCfg_t* data)
*
*   @brief Sets ADC Output Configuration
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - Container for ADC Configuration data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function sets the static device configuration for the data format of the ADC
*   and digital front end output. This is RAW ADC samples of IF signal and needs to
*   be processed by HW accelerator or DSP. The ADC data can be sent to external Processor
*   over High Speed Interface such as LVDS or CSI2. The ADC data size supported are
*   12, 14 and 16 bits and supported formats are Real, Complex 1x and Complex 2x.
*   In Complex 1x, Image band is filtered out and only signal band is sampled in ADC.
*   Where as in Complex 2x, Both Image and Signal band is sampled.\n Complex baseband
*   architecture results in better noise figure and is recommended.

*   @note At the same sampling frequency(Fs), Complex 1x would support IF bandwidth of Fs,
*   where as real and complex 2x would provide IF bandwidth of upto Fs/2.
*/
/* DesignId :  MMWL_DesignId_010 */
/* Requirements : AUTORADAR_REQ-753, AUTORADAR_REQ-754 */
rlReturnVal_t rlSetAdcOutConfig(rlUInt8_t deviceMap, rlAdcOutCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetAdcOutConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetAdcOutConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_ADCOUT_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlAdcOutCfg_t));
    }

    RL_LOGV_ARG0("rlSetAdcOutConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetBpmCommonConfig(rlUInt8_t deviceMap, rlBpmCommonCfg_t* data)
 *
 *   @brief Sets Binary Phase Modulation Common Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for BPM common Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API defines static configurations related to BPM (Binary Phase Modulation) feature in
 *   each of the TXs. E.g. the source of the BPM pattern (one constant value for each chirp as
 *   defined, or intra-chirp pseudo random BPM pattern as found by a programmable
 *   LFSR or a programmable sequence inside each chirp), are defined here.
 *
 *   @note 1: Different source of BPM is currently not supported, hence this API is not required
 *            to be called by application.
 */
/* DesignId : MMWL_DesignId_016 */
/* Requirements : AUTORADAR_REQ-724 */
rlReturnVal_t rlSetBpmCommonConfig(rlUInt8_t deviceMap, rlBpmCommonCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetBpmCommonConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetBpmCommonConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_ADVANCED_FEATURES_SET_MSG,
                                       RL_RF_BPM_COMMON_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlBpmCommonCfg_t));
    }
    RL_LOGV_ARG0("rlSetBpmCommonConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetBpmChirpConfig(rlUInt8_t deviceMap, rlBpmChirpCfg_t* data)
 *
 *   @brief Sets Binary Phase Modulation Chirp Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for BPM chirp configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API defines static configurations related to BPM (Binary Phase Modulation) feature in
 *   each of the TXs
 *
 *   @note 1: BPM values are configured using TX phase shifter and applied at TX start time.
 */
/* DesignId : MMWL_DesignId_037 */
/* Requirements : AUTORADAR_REQ-727 */
rlReturnVal_t rlSetBpmChirpConfig(rlUInt8_t deviceMap, rlBpmChirpCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetBpmChirpConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetBpmChirpConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_ADVANCED_FEATURES_SET_MSG,
                                       RL_RF_BPM_CHIRP_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlBpmChirpCfg_t));
    }

    RL_LOGV_ARG0("rlSetBpmChirpConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetMultiBpmChirpConfig(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                               rlBpmChirpCfg_t** data)
*
*   @brief Sets Binary Phase Modulation configuration for multiple Chirp
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] cnt - number of BPM chirp config data
*   @param[in] data - pointer to linked list/array of BPM chirp configuration data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   Using this API application can configure multiple BPM chirp configuration
*   in a single call. This API defines static configurations related to BPM (Binary
*   Phase Modulation) feature in each of the TXs.
*/
/* DesignId : MMWL_DesignId_122 */
/* Requirements : AUTORADAR_REQ-727 */
rlReturnVal_t rlSetMultiBpmChirpConfig(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                       rlBpmChirpCfg_t **data)
{
    rlReturnVal_t retVal;
    RL_LOGV_ARG0("rlSetMultiBpmChirpConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data) ||
        (cnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};

        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlBpmChirpCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlBpmChirpCfg_t) + (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (cnt <= maxSbCntInMsg)
        {
            sbCntInMsg = cnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = cnt / maxSbCntInMsg;
            lastSbCntInMsg = cnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_ADVANCED_FEATURES_SET_MSG, &inMsg, &inPayloadSb[0]);

        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_ADVANCED_FEATURES_SET_MSG,
                                                            RL_RF_BPM_CHIRP_CONF_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlBpmChirpCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(*data);

                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
                  next profile Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            loopCnt--;

            if (loopCnt == 0U)
            {
                break;
            }
        }
    }
    RL_LOGV_ARG0("rlSetMultiBpmChirpConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetProfileConfig(rlUInt8_t deviceMap, rlUInt16_t cnt, rlProfileCfg_t* data)
 *
 *   @brief Sets Chirp profile Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] cnt  - Number of Profiles
 *   @param[in] data - Array of Profile Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function sets the chirp profile in mmWave Front end. A profile is like a template which
 *   contains coarse information about FMCW signal such as start frequency, chirp slope, chirp
 *   duration, TX power etc. The API allows multiple profiles to be set together by passing the
 *   array of profile data along with count of profiles.
 *
 *   @note 1: One can set upto 4 profiles. Each profile contains coarse inforamtion. Fine dithering
 *            can be added using chirp configuration API
 *   @note 2: This API can be issued dynamically to change profile parameters.
 *            Few parameters which cannot be changed are
 *            1. numAdcSamples
 *            2. digOutSampleRate
 *            3. Programmable filter coefficients
 *   @note 3: Please refer Table \ref chirpCycleTime for details on minimum chirp duration.
 *   @note 4: The max TX output power back-off only up to 20dB is supported.
 *   @note 5: The RF band used in functional chirp profiles shall be within the limit set in
 *            AWR_CAL_MON_FREQUENCY_TX_POWER_LIMITS_SB API.
 *   @note 6: This API takes around 700us to execute in RadarSS sub System
 *   @note 7: Phase shifter(PS) settings are applied in advance at max -5us or at
 *            -(idleTimeConst-1.28us-DfeLagTime) from the knee of the ramp.
 *            If idle time > 6.28us, then PS is applied always at -5us and
 *            if idle time < 6.28us, then PS is applied at -(idleTimeConst-1.28us-DfeLagTime) from
 *            knee of the ramp.
 *            Where DfeLagTime is internal DFE lag time (Please refer rampgen calculator).
 *   @note 8: It is recommended to configure TX start time > -5us or
 *            -(idleTimeConst-1.28us-DfeLagTime) based on PS apply time.
 *
 *   @ref rlSetChirpConfig
 *
 */
/* DesignId : MMWL_DesignId_013 */
/* Requirements :  AUTORADAR_REQ-728, AUTORADAR_REQ-729, AUTORADAR_REQ-730, AUTORADAR_REQ-731,
                   AUTORADAR_REQ-732, AUTORADAR_REQ-733, AUTORADAR_REQ-734, AUTORADAR_REQ-735,
                   AUTORADAR_REQ-736, AUTORADAR_REQ-737, AUTORADAR_REQ-738, AUTORADAR_REQ-739,
                   AUTORADAR_REQ-740, AUTORADAR_REQ-741, AUTORADAR_REQ-742, AUTORADAR_REQ-743,
                   AUTORADAR_REQ-773
 */
rlReturnVal_t rlSetProfileConfig(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                 rlProfileCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetProfileConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data) || (cnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetProfileConfig, Invalid device map\n");
    }
    else
    {
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlProfileCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];
        /* Variable to count message chunks */
        rlUInt16_t indx;
        rlUInt16_t sbCntInAllChunk = 0U;
        rlUInt16_t maxSbInMsg = (RL_CMD_PL_LEN_MAX / (sizeof(rlProfileCfg_t) +
                                                      RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE));

        /* Construct command packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);

        retVal = RL_RET_CODE_OK;
        /* Loop to copy all the command data to one message */
        for (indx = 0U; (indx < cnt) && (retVal == RL_RET_CODE_OK); indx++)
        {
            /* Fill in-message payload */
            /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
              next profile Config */
            /*AR_CODE_REVIEW MR:R.18.4 <APPROVED> "require pointer increment to jump to
              next profile Config */
            /*LDRA_INSPECTED 87 S */
            /*LDRA_INSPECTED 567 S */
            rlDriverFillPayload(RL_RF_DYNAMIC_CONF_SET_MSG, RL_RF_PROFILE_CONF_SB,
                                &inPayloadSb[indx], (rlUInt8_t *)(data + indx),
                                (rlUInt16_t)sizeof(rlProfileCfg_t));

            /* increment Sub-block count in one packet */
            sbCntInAllChunk++;

            /* check if total payload length is going beyond defined limitation */
            if ((maxSbInMsg == (indx + 1U)) || (cnt == sbCntInAllChunk))
            {
                /* setting of numSBC to inMsg */
                inMsg.opcode.nsbc = (indx + 1U);
                /* Send Command to mmWave Radar Device */
                retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            }
        }
    }

    RL_LOGV_ARG0("rlSetProfileConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlGetProfileConfig(rlUInt8_t deviceMap, rlUInt16_t profileId,
 *                                        rlProfileCfg_t* data)
 *
 *   @brief Gets Chirp profile Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] profileId - Profile Id
 *   @param[out] data - Container for Profile Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function gets the FMCW radar chirp properties like FMCW slope, chirp duration,
 *   TX power etc. from the device.
 */
/* DesignId : MMWL_DesignId_118 */
/* Requirements :  AUTORADAR_REQ-728, AUTORADAR_REQ-729, AUTORADAR_REQ-730, AUTORADAR_REQ-731,
AUTORADAR_REQ-732, AUTORADAR_REQ-733, AUTORADAR_REQ-734, AUTORADAR_REQ-735,
AUTORADAR_REQ-736, AUTORADAR_REQ-737, AUTORADAR_REQ-738, AUTORADAR_REQ-739,
AUTORADAR_REQ-740, AUTORADAR_REQ-741, AUTORADAR_REQ-742, AUTORADAR_REQ-743,
AUTORADAR_REQ-773
*/
rlReturnVal_t rlGetProfileConfig(rlUInt8_t deviceMap, rlUInt16_t profileId, rlProfileCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlGetProfileConfig ends...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlGetProfileConfig, Invalid device map\n");
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};
        rlWordParam_t wordParam;
        /* combine half words into WORD structure */
        wordParam.halfWordOne = profileId;
        wordParam.halfWordTwo = 0U;

        /* Construct command packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_GET_MSG, &inMsg, &inPayloadSb);
        /* Fill in-message Payload */
        rlDriverFillPayload(RL_RF_DYNAMIC_CONF_GET_MSG, RL_RF_PROFILE_CONF_SB, &inPayloadSb,
                            (rlUInt8_t *)&wordParam, (rlUInt16_t)sizeof(rlWordParam_t));

        /* Construct response packet */
        rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
        /* Fill in-message Payload */
        rlDriverFillPayload(0U, 0U, &outPayloadSb, (rlUInt8_t *)data, 0U);

        /* Send Command to mmWave Radar Device */
        retVal = rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
    }

    RL_LOGV_ARG0("rlGetProfileConfig ends...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlSetChirpConfig(rlUInt8_t deviceMap, rlUInt16_t cnt, rlChirpCfg_t* data)
*
*   @brief Sets Chirp Configuration
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] cnt  - Number of configurations
*   @param[in] data - Array of Chirp Configuration data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function sets the chirp to chirp variations on top of the chirp profile. The
*   User should first define a profile using rlSetProfileConfig. This function then configures
*   the chirp by associating it with a particular profile defined in rlSetProfileConfig API.
*   In addition to that user can define fine dither to the profile parameters using this API.
*   The dithers used in this configuration are only additive on top of programmed
*   parameters in rlSetProfileConfig. This API allows configuration of 1 or upto 512 chirps.
*   Also it allows configuraiton of which Transmit channels to be used for each chirp.

*   @note 1: One can set upto 512 unique chirps which can be stored in dedicated memory inside
*            mmWave front end. Hence user doesn't need to program the chirps during run time. Also
*            these chirps can be sequenced in a frame using rlSetFrameConfig to create a larger
*            FMCW signal.
*
*   @ref rlSetFrameConfig
*/
/* DesignId : MMWL_DesignId_014 */
/* Requirements : AUTORADAR_REQ-744, AUTORADAR_REQ-745, AUTORADAR_REQ-746, AUTORADAR_REQ-747,
                  AUTORADAR_REQ-748, AUTORADAR_REQ-749, AUTORADAR_REQ-750, AUTORADAR_REQ-751,
                  AUTORADAR_REQ-773
 */
rlReturnVal_t rlSetChirpConfig(rlUInt8_t deviceMap, rlUInt16_t cnt, rlChirpCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetChirpConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data) || (cnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};
        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlChirpCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlChirpCfg_t) + (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (cnt <= maxSbCntInMsg)
        {
            sbCntInMsg = cnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = cnt / maxSbCntInMsg;
            lastSbCntInMsg = cnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);
        /* get the loop count in form of number of msg chunks */
        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_SET_MSG,
                                                            RL_RF_CHIRP_CONF_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlChirpCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(data);
                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to \
                  next chirp Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
            LDRA Tool Issue" */
            /*LDRA_INSPECTED 105 D */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            /* decrement the loop count */
            loopCnt--;
            /* if all chirp configurations have been sent to device then terminate the loop */
            if (loopCnt == 0U)
            {
                break;
            }
        }
    }

    RL_LOGV_ARG0("rlSetChirpConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlGetChirpConfig(rlUInt8_t deviceMap, rlUInt16_t chirpStartIdx,
 *                                      rlUInt16_t chirpEndIdx, rlChirpCfg_t* data)
 *
 *   @brief Gets Chirp Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] chirpStartIdx - Chirp Start Index
 *   @param[in] chirpEndIdx - Chirp End Index
 *   @param[out] data - Container for Chirp Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function gets the chirp configuration from the device.
 */
/* DesignId : MMWL_DesignId_119 */
/* Requirements : AUTORADAR_REQ-744, AUTORADAR_REQ-745, AUTORADAR_REQ-746, AUTORADAR_REQ-747,
AUTORADAR_REQ-748, AUTORADAR_REQ-749, AUTORADAR_REQ-750, AUTORADAR_REQ-751,
AUTORADAR_REQ-773
*/
rlReturnVal_t rlGetChirpConfig(rlUInt8_t deviceMap, rlUInt16_t chirpStartIdx,
                               rlUInt16_t chirpEndIdx, rlChirpCfg_t *data)
{
    rlReturnVal_t retVal;
    rlChirpCfg_t *chirpCfgData = data;

    RL_LOGV_ARG0("rlGetChirpConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == chirpCfgData))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlGetChirpConfig, Invalid device map\n");
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntOutMsg, sbCntOutMsg, numChnkOfMsg, lastSbCntOutMsg, loopCnt;

        /* file chirp start and end index */
        rlWordParam_t wordParam;

        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb;

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlChirpCfg_t));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntOutMsg = (RL_CMD_PL_LEN_MAX - (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE)) / sbLen;

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if ((chirpEndIdx - chirpStartIdx + 1U) <= maxSbCntOutMsg)
        {
            sbCntOutMsg = (chirpEndIdx - chirpStartIdx + 1U);
            numChnkOfMsg = 0U;
            lastSbCntOutMsg = 0U;
        }
        else
        {
            sbCntOutMsg = maxSbCntOutMsg;
            numChnkOfMsg = (chirpEndIdx - chirpStartIdx + 1U) / maxSbCntOutMsg;
            lastSbCntOutMsg = (chirpEndIdx - chirpStartIdx + 1U) % maxSbCntOutMsg;
        }
        /* set chirp start index to inMsg field */
        /* AR_CODE_REVIEW MR:R.2.2 <APPROVED> "required to intial with startIdx" */
        /*LDRA_INSPECTED 8 D */
        wordParam.halfWordTwo = chirpStartIdx;

        /* get the loop count in form of number of msg chunks */
        loopCnt = numChnkOfMsg + 1U;
        /* setting num of sub-block to one in outMsg */
        outMsg.opcode.nsbc = 1U;

        while (retVal == RL_RET_CODE_OK)
        {
            /* combine half words into WORD structure */
            wordParam.halfWordOne = wordParam.halfWordTwo;
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntOutMsg != 0U))
            {
                wordParam.halfWordTwo = wordParam.halfWordOne + (lastSbCntOutMsg - 1U);
            }
            else
            {
                wordParam.halfWordTwo = wordParam.halfWordOne + (sbCntOutMsg - 1U);
            }

            /* Construct command packet */
            rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_GET_MSG, &inMsg, &inPayloadSb);
            /* Fill in-message Payload */
            rlDriverFillPayload(RL_RF_DYNAMIC_CONF_GET_MSG, RL_RF_CHIRP_CONF_SB, &inPayloadSb,
                                (rlUInt8_t *)&wordParam, (rlUInt16_t)sizeof(rlWordParam_t));

            /* Set Command Sub Block*/
            outPayloadSb.sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_GET_MSG,
                                                   RL_RF_CHIRP_CONF_SB);
            outPayloadSb.len = (rlUInt16_t)sizeof(rlChirpCfg_t);
            outPayloadSb.pSblkData = (rlUInt8_t *)(chirpCfgData);

            /* Construct response packet */
            rlDriverConstructOutMsg(outMsg.opcode.nsbc, &outMsg, &outPayloadSb);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);

            /* increment data pointer for next chunk of reponse */
            chirpCfgData += sbCntOutMsg;

            /* decrement the loop count */
            loopCnt--;
            wordParam.halfWordTwo++;
            /* if all chirp configurations have been sent to device then terminate the loop */
            if (loopCnt == 0U)
            {
                break;
            }
        }
    }

    RL_LOGV_ARG0("rlGetChirpConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetMultiChirpCfg(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                         rlChirpCfg_t **data)
*
*   @brief Injects chirp configuration to be programmed dynamically
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] cnt - number of chirps
*   @param[in] data - Pointer to Chirp configuration linked list/array
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function sets the chirp to chirp variations on top of the chirp profile. The
*   User should first define a profile using rlSetProfileConfig.\n This function then configures
*   the chirp by associating it with a particular profile defined in rlSetProfileConfig API.
*   In addition to that user can define fine dither to the profile parameters using this API \n
*   This API allows configuration of 1 or upto 512 chirps. Also it allows configuraiton of
*   which Transmit channels to be used for each chirp.
*
*   @note One can set upto 512 unique chirps which can be stored in dedicated memory inside
*   mmWave front end. Hence user doesn't need to program the chirps during run time. Also these
*   chirps can be sequenced in a frame using rlSetFrameConfig to create a larger FMCW signal\n
*   This API is similar to rlSetChirpConfig but gives the flexibility to pass the array of
*   chirp configuration pointers, so chirp configuration memory need not be contiguous.
*
*/
rlReturnVal_t rlSetMultiChirpCfg(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                 rlChirpCfg_t **data)
{
    rlReturnVal_t retVal;
    RL_LOGV_ARG0("rlSetMultiChirpCfg starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data) ||
        (cnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};

        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlChirpCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlChirpCfg_t) + (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (cnt <= maxSbCntInMsg)
        {
            sbCntInMsg = cnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = cnt / maxSbCntInMsg;
            lastSbCntInMsg = cnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);

        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_SET_MSG,
                                                            RL_RF_CHIRP_CONF_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlChirpCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(*data);

                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
                  next profile Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            loopCnt--;

            if (loopCnt == 0U)
            {
                break;
            }
        }
    }
    RL_LOGV_ARG0("rlSetMultiChirpCfg ends...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlSetFrameConfig(rlUInt8_t deviceMap, rlFrameCfg_t* data)
 *
 *   @brief Sets Frame Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for Frame Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function allows configuration of FMCW frame in mmWave Front end. A Frame is basically
 *   a sequence of chirps and how this sequnece needs to be repeated over time. User first need
 *   to define a profile and set of chirps(associated with a profile). \n This function then
 *   defines how to sequence these chirps. The same chirp can be simply looped to create a large
 *   FMCW frame or multiple unique chirps cane be sequenced to create the frame. Chirp Start and
 *   end Index defines how to sequence them in a frame. \n The API also allows configuration of
 *   number of frames to be transmitted, periodicity of the frame and the trigger method. The
 *   trigger method could be SW API based trigger or HW SYNC IN based trigger. \n This API calls
 *   internally two APIs, one to RadraSS for sensor configuraion and another to MasterSS for
 *   datapath configuraion. \n
 *
 *   @note 1: If hardware triggered mode is used, the SYNC_IN pulse width should be less than the ON
 *            time of the frame (in case of legacy frame config mode) or the ON time of the burst
 *            (in case of advanced frame config mode). Also, the minimum pulse width of SYNC_IN
 *            should be 25 ns.
 *   @note 2: If frame trigger delay is used with hardware triggered mode, then external SYNC_IN
 *            pulse periodicity should take care of the configured frame trigger delay and frame
 *            periodicity. The external pulse should be issued only after the sum total of frame
 *            trigger delay and frame periodicity.
 *   @note 3: If dummy chirp is used then programmer should make sure the idle time of dummy
 *            chirp >= 4us + DFE spill over time of previous chirp (calculate from rampgen
 *            calculator). The first chirp of frame cannot be a dummy chirp.
 *   @note 4: In Hw triggered mode, the Hw pulse should be issued or periodicity of pulse is
 *            configured such that, the pulse is generated only 150us after the completion of
 *            previous frame/burst (The pulse should not be issued before end of previous frame/
 *            burst). The time delta between end of previous frame/burst and raising edge of HW
 *            pulse recommended to be < 300us.
 *   @note 5: Frame could have multiple chirps associated with different profile, but number of
 *            samples need to be same in all the profiles. \n
 *   @note 6: The PF_NUM_ADC_SAMPLES parameter should be identical across chirps in a frame,
 *            when multiple profiles are used in a frame. \n
 *   @note 7: The PF_DIGITAL_OUTPUT_SAMPLING_RATE impacts the LVDS/CSI2 data rate in a frame, so it
 *            is recommended to analyze timing impact if different sample rate is used across
 *            chirps in a frame. \n
 *   @note 8: Please refer Table \ref interSubFrameTime for details on minimum inter-frame blank
 *            time requirements. \n
 *   @note 9: If advance chirp configuration is enabled then this API takes around 1.8ms to
 *            execute in RadarSS sub System for 128 chirps. The error checks for each parameters
 *            of advance chirp is done in frame configuration API. This option can be disabled by
 *            using ADVANCE_CHIRP_ERROR_CHK_DIS option in \ref rlRfSetMiscConfig API. \n
 */
/* DesignId : MMWL_DesignId_015 */
/* Requirements : AUTORADAR_REQ-715, AUTORADAR_REQ-716, AUTORADAR_REQ-717,
   AUTORADAR_REQ-718, AUTORADAR_REQ-719, AUTORADAR_REQ-720 */
rlReturnVal_t rlSetFrameConfig(rlUInt8_t deviceMap, rlFrameCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetFrameConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetFrameConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_FRAME_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlFrameCfg_t));

        /* If only mmWaveLink instance is running on Host */
        if ((retVal == RL_RET_CODE_OK) && (RL_PLATFORM_HOST == rlDriverGetPlatformId()))
        {
            rlFrameApplyCfg_t frameApplyCfgArgs;
            rlUInt32_t tempVar;
            frameApplyCfgArgs.reserved = 0U;
            frameApplyCfgArgs.numAdcSamples = (data->numAdcSamples);
            /*AR_CODE_REVIEW MR:R.10.8 <APPROVED> "Conversion required for RL_SWAP_32 operation"*/
            /*LDRA_INSPECTED 332 S */
            tempVar = (rlUInt32_t)(((data->chirpEndIdx - data->chirpStartIdx) + 1U) *
                                   data->numLoops);
#ifndef MMWL_BIG_ENDIAN
            frameApplyCfgArgs.numChirps = tempVar;
#else
            frameApplyCfgArgs.numChirps = RL_SWAP_32(tempVar);
#endif
            retVal += rlDeviceFrameConfigApply(deviceMap, &frameApplyCfgArgs);
        }
    }

    RL_LOGV_ARG0("rlSetFrameConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlGetFrameConfig(rlUInt8_t deviceMap, rlFrameCfg_t* data)
 *
 *   @brief Gets Frame Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[out] data - Container for Frame Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function reads the frame properties of the device. This API calls internally two APIs,
 *   one to RadraSS for sensor configuraion and another to MasterSS for datapath configuraion. \n
 */
/* DesignId : MMWL_DesignId_120 */
/* Requirements : AUTORADAR_REQ-715, AUTORADAR_REQ-716, AUTORADAR_REQ-717,
AUTORADAR_REQ-718, AUTORADAR_REQ-719, AUTORADAR_REQ-720 */
rlReturnVal_t rlGetFrameConfig(rlUInt8_t deviceMap, rlFrameCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlGetFrameConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlGetFrameConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_DYNAMIC_CONF_GET_MSG,
                                       RL_RF_FRAME_CONF_SB, (rlUInt8_t *)data, 0U);

        /* If only mmWaveLink instance is running on Host */
        if ((retVal == RL_RET_CODE_OK) && (RL_PLATFORM_HOST == rlDriverGetPlatformId()))
        {
            /* Initialize Command and Response Sub Blocks */
            rlDriverMsg_t inMsg = {0};
            rlDriverMsg_t outMsg = {0};
            rlPayloadSb_t inPayloadSb = {0};
            rlPayloadSb_t outPayloadSb = {0};
            rlFrameApplyCfg_t frameApplyCfgArgs = {0};

            /* Construct command packet */
            rlDriverConstructInMsg(RL_DEV_STATUS_GET_MSG, &inMsg, &inPayloadSb);
            /* Fill in-message Payload */
            /*AR_CODE_REVIEW MR:R.11.2 <APPROVED> "Need to pass NULL pointer" */
            /*LDRA_INSPECTED 95 S */
            rlDriverFillPayload(RL_DEV_STATUS_GET_MSG, RL_SYS_FRAME_CONFIG_GET_SB,
                                &inPayloadSb, (rlUInt8_t *)RL_NULL_PTR, 0x0U);

            /* Construct response packet */
            rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
            /* Fill out-message Payload */
            rlDriverFillPayload(0U, 0U, &outPayloadSb,
                                (rlUInt8_t *)&frameApplyCfgArgs, 0U);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
#ifndef MMWL_BIG_ENDIAN

            /* update parameter received by Get Command */
            data->numAdcSamples = frameApplyCfgArgs.numAdcSamples;
#else
            /* update parameter received by Get Command */
            data->numAdcSamples = RL_SWAP_32(frameApplyCfgArgs.numAdcSamples);
#endif
        }
    }

    RL_LOGV_ARG0("rlGetFrameConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetAdvFrameConfig(rlUInt8_t deviceMap, rlAdvFrameCfg_t* data)
 *
 *   @brief Sets Advance Frame Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for Advance Frame Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function allows configuration of advance frame in mmWave Front end. Advance Frame is
 *   a sequence of chirps and how this sequnece needs to be repeated over time. User first need
 *   to define a profile and set of chirps(associated with a profile).\n This function then defines
 *   how to sequence these chirps. Multiple chirps can be looped to create a burst. Multiple bursts
 *   can be grouped to create a sub-frame. Multiple sub-frames(Upto 4) can be grouped to create
 *   advance frame. \n This function defines the advance frame properties like the number of burst
 *   in subframe, number of chirps and loops in a burst, sequence of subframes to be transmitted,
 *   number of frames to be transmitted, periodicity of the frame and the trigger method. This API
 *   calls internally two APIs, one to RadraSS for sensor configuraion and another to MasterSS for
 *   datapath configuraion. \n
 *
 *   @note 1: If hardware trigger mode is used with subFrameTrigger = 0, then the trigger should be
 *            issued for each burst. If subFrameTrigger = 1, then the trigger needs to be issued
 *            for each sub-frame.
 *   @note 2: If hardware triggered mode is used, the SYNC IN pulse width should be less than the
 *            ON time of the frame (in case of legacy frame config mode) or the ON time of the
 *            burst (in case of advanced frame config mode). Also, the minimum pulse width of
 *            SYNC IN should be 25 ns.
 *   @note 3: If frame trigger delay is used with hardware triggered mode, then external SYNC IN
 *            pulse periodicity should take care of the configured frame trigger delay and frame
 *            periodicity. The external pulse should be issued only after the sum total of frame
 *            trigger delay and frame periodicity.
 *   @note 4: In Hw triggered mode, the Hw pulse should be issued or periodicity of pulse is
 *            configured such that, the pulse is generated only 150us after the completion of
 *            previous frame/burst (The pulse should not be issued before end of previous frame/
 *            burst). The time delta between end of previous frame/burst and raising edge of Hw
 *            pulse recommended to be < 300us.
 *   @note 5: The PF_NUM_ADC_SAMPLES parameter should be identical across chirps in a sub-frame,
 *            when multiple profiles are used in a sub-frame. \n
 *   @note 6: The PF_DIGITAL_OUTPUT_SAMPLING_RATE impacts the LVDS/CSI2 data rate in a sub-frame,
 *            so it is recommended to analyze timing impact if different sample rate is used across
 *            chirps in a sub-frame. \n
 *   @note 7: Please refer Table \ref interBurstTime and \ref interSubFrameTime for details on
 *            minimum inter-frame blank time requirements. \n
 *   @note 8: If advance chirp configuration is enabled then this API takes around 1.8ms to
 *            execute in RadarSS sub System for 128 chirps. The error checks for each parameters
 *            of advance chirp is done in frame configuration API. This option can be disabled by
 *            using ADVANCE_CHIRP_ERROR_CHK_DIS option in \ref rlRfSetMiscConfig API. \n
 */
/* DesignId : MMWL_DesignId_046 */
/* Requirements : AUTORADAR_REQ-795 */
rlReturnVal_t rlSetAdvFrameConfig(rlUInt8_t deviceMap, rlAdvFrameCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetAdvFrameConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetAdvFrameConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_FRAME_CONF_ADVANCED_SB,
                                       (rlUInt8_t *)(&(data->frameSeq)),
                                       (rlUInt16_t)sizeof(rlAdvFrameSeqCfg_t));

        /* If only mmWaveLink instance is running on Host */
        if ((RL_RET_CODE_OK == retVal) && (RL_PLATFORM_HOST == rlDriverGetPlatformId()))
        {
            retVal += rlDeviceAdvFrameConfigApply(deviceMap, &(data->frameData));
        }
    }

    RL_LOGV_ARG0("rlSetAdvFrameConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlGetAdvFrameConfig(rlUInt8_t deviceMap, rlAdvFrameCfg_t* data)
 *
 *   @brief Gets Advance Frame Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[out] data - Container for Advance Frame Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function reads the advance frame properties of the device. This API calls internally two
 *   APIs, one to RadraSS for sensor configuraion and another to MasterSS for datapath
 *   configuraion. \n
 */
/* DesignId : MMWL_DesignId_047 */
/* Requirements : AUTORADAR_REQ-796 */
rlReturnVal_t rlGetAdvFrameConfig(rlUInt8_t deviceMap, rlAdvFrameCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlGetAdvFrameConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlGetAdvFrameConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_DYNAMIC_CONF_GET_MSG,
                                       RL_RF_FRAME_CONF_ADVANCED_SB,
                                       (rlUInt8_t *)(&(data->frameSeq)), 0U);

        /* If only mmWaveLink instance is running on Host */
        if ((RL_RET_CODE_OK == retVal) && (RL_PLATFORM_HOST == rlDriverGetPlatformId()))
        {
            /* Initialize Command and Response Sub Blocks */
            rlDriverMsg_t inMsg = {0};
            rlDriverMsg_t outMsg = {0};
            rlPayloadSb_t inPayloadSb = {0};
            rlPayloadSb_t outPayloadSb = {0};

            /* Construct command packet */
            rlDriverConstructInMsg(RL_DEV_STATUS_GET_MSG, &inMsg, &inPayloadSb);
            /* Fill in-message Payload */
            /*AR_CODE_REVIEW MR:R.11.2 <APPROVED> "Need to pass NULL pointer" */
            /*LDRA_INSPECTED 95 S */
            rlDriverFillPayload(RL_DEV_STATUS_GET_MSG,
                                RL_SYS_ADV_FRAME_CONFIG_GET_SB,
                                &inPayloadSb, (rlUInt8_t *)RL_NULL_PTR, 0U);

            /* Construct response packet */
            rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
            /* Fill out-message Payload */
            rlDriverFillPayload(0U, 0U, &outPayloadSb,
                                (rlUInt8_t *)(&(data->frameData)), 0U);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
        }
    }

    RL_LOGV_ARG0("rlGetAdvFrameConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetContModeConfig(rlUInt8_t deviceMap, rlContModeCfg_t* data)
 *
 *   @brief Sets Continous mode Configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Continous mode Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function sets the FMCW radar continous mode properties like Start Freq, TX power
 *   etc. In continuous mode, the signal is not frequency modulated but has the same frequency
 *   over time.
 *
 *   @note : The continuous streaming mode configuration APIs are supported only for debug purpose.
 *           Please refer latest DFP release note for more info.
 */
/* DesignId : MMWL_DesignId_038 */
/* Requirements : AUTORADAR_REQ-827 */
rlReturnVal_t rlSetContModeConfig(rlUInt8_t deviceMap, rlContModeCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetContModeConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetContModeConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_CONT_STREAMING_MODE_CONF_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlContModeCfg_t));
    }
    RL_LOGV_ARG0("rlSetContModeConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlEnableContMode(rlUInt8_t deviceMap, rlContModeEn_t* data)
 *
 *   @brief Enable/Disable Continous mode
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Continous Mode enable/disable
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function enables/disables the FMCW radar continous mode
 */
/* DesignId : MMWL_DesignId_039 */
/* Requirements : AUTORADAR_REQ-828 */
rlReturnVal_t rlEnableContMode(rlUInt8_t deviceMap, rlContModeEn_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlEnableContMode starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlEnableContMode, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_CONT_STREAMING_MODE_EN_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlContModeEn_t));
    }

    RL_LOGV_ARG0("rlEnableContMode starts...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetLowPowerModeConfig(rlUInt8_t deviceMap, rlLowPowerModeCfg_t* data)
*
*   @brief Sets Low Power Mode Configuration
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - Container for Low power mode Configuration data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function sets the static device configurations of low power options. Sigma Delta ADC
*   root sampling reduces to half the rate to save power in small IF bandwidth applications.
*
*   @note : Low power ADC mode is mandatory on 5 MHz part variant(for e.g. xWR1642), Normally
            if IF band width <= 7.5MHz then low power mode setting is recommended.
*/
/* DesignId :  MMWL_DesignId_011 */
/* Requirements : AUTORADAR_REQ-755 */
rlReturnVal_t rlSetLowPowerModeConfig(rlUInt8_t deviceMap, rlLowPowerModeCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetLowPowerModeConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetLowPowerModeConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_LOWPOWERMODE_CONF_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlLowPowerModeCfg_t));
    }

    RL_LOGV_ARG0("rlSetLowPowerModeConfig ends...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlSensorStart(rlUInt8_t deviceMap)
 *
 *   @brief Triggers Transmission of Frames
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function triggers the transmission of the frames as per the frame and chirp configuration
 *   If trigger mode is selected as SW API based trigger, mmWaveFront end would start chirp
 *   immediately after receiving this API. If trigger mode is HW SYNC IN pulse, it would wait for
 *   SYNC pulse
 *
 *   @note 1: Once the chirping starts, mmWave Front end would send asynchronous event
 *           RL_RF_AE_FRAME_TRIGGER_RDY_SB indicating the start of frame
 *   @note 2: This API might get deprecated in future releases. Refer to \ref rlFrameStartStop API
 */
/* DesignId : MMWL_DesignId_017 */
/* Requirements : AUTORADAR_REQ-759 */
rlReturnVal_t rlSensorStart(rlUInt8_t deviceMap)
{
    rlReturnVal_t retVal;

    rlFrameTrigger_t frameTriggerArgs;

    RL_LOGV_ARG0("rlSensorStart starts...\n");

    frameTriggerArgs.startStop = RL_FRAME_TRIGGER_START;
    frameTriggerArgs.reserved = 0U;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSensorStart, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_FRAME_TRIG_MSG,
                                       RL_RF_FRAMESTARTSTOP_CONF_SB,
                                       (rlUInt8_t *)(&frameTriggerArgs),
                                       (rlUInt16_t)sizeof(rlFrameTrigger_t));
    }
    RL_LOGV_ARG0("rlSensorStart ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSensorStop(rlUInt8_t deviceMap)
 *
 *   @brief Stops Transmission of Frames
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function stops the transmission of the frames.
 *
 *   @note 1: Once the chirping stops, mmWave Front end would send asynchronous event
 *            RL_RF_AE_FRAME_END_SB indicating the stop of frame
 *   @note 2: When Frame Stop command is sent to RadarSS, the frame will be stopped after
 *            completing all the chirps of a Frame/Advance frame. In non periodic HW triggered
 *            mode if frame stop command is issued when HW pulses are paused (i.e after completing
 *            previous frame) then a HW pulse is required to trigger next frame/bursts and frame
 *            will be stopped at the end of this triggered frame. In HW triggered mode, the forced
 *            frame stop is not supported, the frame end command is honored only if frames are
 *            actively running.
 *   @note 3: This API might get deprecated in future releases. Refer to \ref rlFrameStartStop API
 */
/* DesignId : MMWL_DesignId_018 */
/* Requirements : AUTORADAR_REQ-760 */
rlReturnVal_t rlSensorStop(rlUInt8_t deviceMap)
{
    rlReturnVal_t retVal;

    rlFrameTrigger_t frameTriggerArgs;

    frameTriggerArgs.startStop = RL_FRAME_TRIGGER_STOP;
    frameTriggerArgs.reserved = 0U;

    RL_LOGV_ARG0("rlSensorStop starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSensorStop, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_FRAME_TRIG_MSG,
                                       RL_RF_FRAMESTARTSTOP_CONF_SB,
                                       (rlUInt8_t *)(&frameTriggerArgs),
                                       (rlUInt16_t)sizeof(rlFrameTrigger_t));
    }
    RL_LOGV_ARG0("rlSensorStop ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlFrameStartStop(rlUInt8_t deviceMap, rlFrameTrigger_t* data)
 *
 *   @brief Triggers/Stops Transmission of Frames
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for Frame trigger Configuration data
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function triggers/stops the transmission of the frames as per the frame and chirp
 *   configuration. If trigger mode is selected as SW API based trigger, mmWaveFront end would
 *   start chirp immediately after receiving this API. If trigger mode is HW SYNC IN pulse, it
 *   would wait for SYNC pulse
 *
 *   @note 1: Once the chirping starts, mmWave Front end would send asynchronous event
 *            RL_RF_AE_FRAME_TRIGGER_RDY_SB indicating the start of frame
 *   @note 2: Once the chirping stops, mmWave Front end would send asynchronous event
 *            RL_RF_AE_FRAME_END_SB indicating the stop of frame
 *   @note 3: When Frame Stop command with 'option-0' is sent to RadarSS, the frame will be stopped
 *            after completing all the chirps of a Frame/Advance frame (sub-frames). \n
 *   @note 4: In non periodic Hw triggered mode or in sub-frame triggered mode, if frame needs to
 *            be stopped immediately then frame stop command with 'option-4' can be used. The
 *            'option-4' can not be used when active frames are running. \n
 *   @note 5: Recommended to re-issue frame configuration API if frame is not stopped at sub-frame
 *            boundary, this is to re-config CSI2 or LVDS data path configuration in MSS. \n
 */
/* DesignId : MMWL_DesignId_133 */
/* Requirements : AUTORADAR_REQ-1059 */
rlReturnVal_t rlFrameStartStop(rlUInt8_t deviceMap, rlFrameTrigger_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlFrameStartStop starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlFrameStartStop, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_FRAME_TRIG_MSG,
                                       RL_RF_FRAMESTARTSTOP_CONF_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlFrameTrigger_t));
    }
    RL_LOGV_ARG0("rlFrameStartStop ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfInit(rlUInt8_t deviceMap)
 *
 *   @brief Initializes the RF/Analog Subsystem
 *
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   Initializes the RF/Analog Subsystem. This triggers one time calibrations for APLL
 *   and synthesizer. Calibration can be enabled/disabled using Calibraton configuration APIs.
 *   Device will turn on Tx/Rx and components for the calibration which will cause higher curent
 *   consumption momenterily.
 *
 *   @note 1: Once the calibration is complete, mmWave Front end would send asynchronous event
 *            RL_RF_AE_INITCALIBSTATUS_SB indicating the result of the initialization/calibrations.
 *            Application needs to wait for this Async event message before calling next APIs.
 *   @note 2: The following boot-time calibrations are susceptible to corruption by interference.
 *            The calibrations may result in false configuration of the RF analog sections due to
 *            corruption by interference during the calibration measurements. \n
 *            a. RX gain calibration (susceptible to interference) \n
 *            b. RX IQMM calibration (susceptible to interference) \n
 *            c. TX Phase calibration (susceptible to interference) \n
 *            It is recommended to perform factory calibration and store the calibration data in
 *            non volatile memory using rlRfCalibDataStore. This data can be restored to radar
 *            device using rlRfCalibDataRestore API. \n
 *   @note 3: It is not recommended to issue this API in runtime multiple times. This API shall be
 *            issued only once after power cycle with or without calibration data restore operation.
 */
/* DesignId : MMWL_DesignId_012 */
/* Requirements : AUTORADAR_REQ-758 */
rlReturnVal_t rlRfInit(rlUInt8_t deviceMap)
{
    /* Initialize Command and Response Sub Blocks */
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfInit starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlRfInit, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_INIT_MSG, RL_RF_RF_INIT_SB, NULL, 0U);
    }
    RL_LOGV_ARG0("rlRfInit ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetTestSourceConfig(rlUInt8_t deviceMap, rlTestSource_t* data)
*
*   @brief Configures the Test Source
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - Container for Test source configuration
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API allows configuration of the Test Source in mmWave Front end. A Test source simulates
*   2 objects at certain position relative to the mmWave device and generates the RAW ADC data.
*   It also simulates velocity of objects, relative position of TX and RX antennas.

*   @note 1: This helps in checking the integrity of control and data path during development
*            phase. API is meant to be used in development phase only and doesn't relate to any
*            real use case.
*   @note 2: The test source configuration APIs are supported only for debug purpose. Please refer
*            latest DFP release note for more info.
*   @note 3: After test source usage, it is recommend to disable the test source and issue profile
*            configuration API again for normal functionality of radar. \n
*/
/* DesignId : MMWL_DesignId_035 */
/* Requirements : AUTORADAR_REQ-790, AUTORADAR_REQ-1008 */
rlReturnVal_t rlSetTestSourceConfig(rlUInt8_t deviceMap, rlTestSource_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetTestSourceConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetTestSourceConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_TEST_SOURCE_CONFIG_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlTestSource_t));
    }
    RL_LOGV_ARG0("rlSetTestSourceConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlTestSourceEnable(rlUInt8_t deviceMap, rlTestSourceEnable_t* data)
 *
 *   @brief Enables the Test Source
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Container for Test source enable parameters
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   Enables the Test Source that is configured using \ref rlSetTestSourceConfig API
 */
/* DesignId : MMWL_DesignId_036 */
/* Requirements : AUTORADAR_REQ-791 */
rlReturnVal_t rlTestSourceEnable(rlUInt8_t deviceMap, rlTestSourceEnable_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlTestSourceEnable starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlTestSourceEnable, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_TEST_SOURCE_ENABLE_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlTestSourceEnable_t));
    }
    RL_LOGV_ARG0("rlTestSourceEnable ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfGetTemperatureReport(rlUInt8_t deviceMap, rlRfTempData_t* data)
 *
 *   @brief Gets Time and Temperature information report
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[out] data - Structure to store temperature report from all the temp sensors
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function reads Temperature information from all temperature sensors in the device
 */
/* DesignId : MMWL_DesignId_029 */
/* Requirements : AUTORADAR_REQ-789 */
rlReturnVal_t rlRfGetTemperatureReport(rlUInt8_t deviceMap, rlRfTempData_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfGetTemperatureReport starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlRfGetTemperatureReport, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_MISC_CONF_GET_MSG,
                                       RL_RF_CHAR_CONF_DYNAMIC_GET_SB,
                                       (rlUInt8_t *)data, (rlUInt16_t)sizeof(rlRfTempData_t));
    }
    RL_LOGV_ARG0("rlRfGetTemperatureReport ends...\n");

    return retVal;
}
/** @fn rlReturnVal_t rlRfDfeRxStatisticsReport(rlUInt8_t deviceMap, rlDfeStatReport_t* data)
 *
 *   @brief Gets Digital Front end statistics such as Residual DC, RMS power in I and
 *          Q chains for different Receive channels for different selected profiles.
 *          It also includes Cross correlation between I and Q chains
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data      - Container of dfe receiver status report
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   Gets Digital Front end statistics such as Residual DC, RMS power in I and
 *   Q chains for different Receive channels for different selected profiles.
 *   It also includes Cross correlation between I and Q chains
 */
/* DesignId : MMWL_DesignId_041 */
/* Requirements : AUTORADAR_REQ-788 */
rlReturnVal_t rlRfDfeRxStatisticsReport(rlUInt8_t deviceMap, rlDfeStatReport_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfDfeRxdStatisticsReport starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_MONITORING_REPO_GET_MSG,
                                       RL_RF_DFE_STATISTICS_REPORT_GET_SB,
                                       (rlUInt8_t *)data, 0U);
    }
    RL_LOGV_ARG0("rlRfDfeRxdStatisticsReport completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfDynamicPowerSave(rlUInt8_t deviceMap, rlDynPwrSave_t* data)
 *
 *   @brief : Configure dynamic power saving feature.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data      - Container of dynamic power save information
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   Configure dynamic power saving feature during Inter chirp Idle time by
 *   turning off various circuits such as Transmitter, Receiver and LO distribution
 *   blocks
 *   @note 1: whether to enable dynamic power saving during inter-chirp IDLE times by turning off
 *            various circuits e.g. TX, RX, LO Distribution blocks. If Idle time + Tx start
 *            time < 10us or Idle time < 3.5us then inter-chirp dynamic power save option will be
 *            disabled, in that case, 15us of inter-burst idle time will be utilized to configure
 *            sequencer LO, TX and RX signal timings by firmware.
 *   @note 2: All the 3 configuration bits (TX, RX and LO) should have same value, i.e. user
 *            should program value 0x7 to enable power save or 0x0 to disable the power save
 *            in blkCfg.
 */
/* DesignId : MMWL_DesignId_019 */
/* Requirements : AUTORADAR_REQ-797 */
rlReturnVal_t rlRfDynamicPowerSave(rlUInt8_t deviceMap, rlDynPwrSave_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfDynamicPowerSave starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_DYNAMICPOWERSAVE_CONF_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlDynPwrSave_t));
    }
    RL_LOGV_ARG0("rlRfDynamicPowerSave completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetDeviceCfg(rlUInt8_t deviceMap, rlRfDevCfg_t* data)
 *
 *   @brief : Set different RadarSS device configurations
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data      - Configuration parameter for AE.
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   Set different RadarSS device configurations. Enable and Configure asynchronous event
 *   direction for device. By default all asynchronous event are enabled and sent to
 *   the platform which issued the API. Below events can be configured to be received
 *   on different platform by using this API:\n
 *   [1.] CPU_FAULT [2.] ESM_FAULT [3.] ANALOG_FAULT
 *   Similarly all monitoring events can be configured to be received on specific platform
 *   using this API
 *   Below events can be disabled using this API:\n
 *   [1.] FRAME_START_ASYNC_EVENT [2.] FRAME_STOP_ASYNC_EVENT \n
 *   Enable[1]/Disable[0] RadarSS Watchdog, where by default it is disable. Configure CRC type
 *   for asynchronous event from RadarSS  [0] 16Bit, [1] 32Bit, [2] 64Bit.
 */
/* DesignId : MMWL_DesignId_042 */
/* Requirements : AUTORADAR_REQ-794 */
rlReturnVal_t rlRfSetDeviceCfg(rlUInt8_t deviceMap, rlRfDevCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetDeviceCfg starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_DEVICE_CFG_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfDevCfg_t));
    }
    RL_LOGV_ARG0("rlRfSetDeviceCfg completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetGpAdcConfig(rlUInt8_t deviceMap, rlGpAdcCfg_t* data)
 *
 *   @brief : Configure GP ADC data parameters
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data      - Configuration parameter for GP ADC
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API enables the GPADC reads for external inputs (available only in xWR1642/xWR6843/
 *   xWR1843). xWR1642/xWR1843 sends GP-ADC measurement data in async event
 *   RL_RF_AE_GPADC_MEAS_DATA_SB
 *
 *   @note : The actual measurement of these GPADC signal are done in inter-burst or frame idle
 *           time and the result AE sub block will be sent only after completing all the
 *           measurements.
 */
/* DesignId : MMWL_DesignId_044 */
/* Requirements : AUTORADAR_REQ-792 */
rlReturnVal_t rlSetGpAdcConfig(rlUInt8_t deviceMap, rlGpAdcCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetGpAdcConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_GPADC_CONF_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlGpAdcCfg_t));
    }
    RL_LOGV_ARG0("rlSetGpAdcConfig completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetLdoBypassConfig(rlUInt8_t deviceMap, rlRfCalibDisableCfg_t* data)
 *
 *   @brief Enables/Disables LDO bypass mode. By default Internal LDO is enabled
 *          and 1.3V supply is required for RF. If external PMIC supplies 1.0V RF
 *          Supply directly, Internal LDO has to be bypassed using this API.
 *   @param[out] data - LDO enable/disable configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function Enables/Disables LDO bypass mode.
 *   @note : Refer to EVM/Board user guide to understand the Power Supply scheme before using this
 *           API. If 1.3 V RF supply is provided and this API is called to enable LDO bypass mode,
 *           devive could burn. Typically in TI EVMs, PMIC is configured to supply 1.3V to the RF
 *           supplies, so in that case don't bypass LDO.
 */
/* DesignId : MMWL_DesignId_043 */
/* Requirements : AUTORADAR_REQ-793 */
rlReturnVal_t rlRfSetLdoBypassConfig(rlUInt8_t deviceMap, rlRfLdoBypassCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetLdoBypassConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_LDOBYPASS_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfLdoBypassCfg_t));
    }

    RL_LOGV_ARG0("rlRfSetLdoBypassConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfSetPhaseShiftConfig(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                                rlRfPhaseShiftCfg_t* data)
*
*   @brief Enable/Disable phase shift configurations per chirp in each of the TXs
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] cnt  - Number of configurations
*   @param[in] data - phase shift enable/disable configuration
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function configures the static phase shift configurations per chirp in each of the
*   TXs. This API is applicable only in certain devices (please refer data sheet). This API
*   will be honored after enabling per chirp phase shifter in rlRfSetMiscConfig.
*
*   @note 1: Phase shifter(PS) settings are applied in advance at max -5us or at
*            -(idleTimeConst-1.28us-DfeLagTime) from the knee of the ramp.
*            If idle time > 6.28us, then PS is applied always at -5us and
*            if idle time < 6.28us, then PS is applied at -(idleTimeConst-1.28us-DfeLagTime) from
*            knee of the ramp.
*            Where DfeLagTime is internal DFE lag time (Please refer rampgen calculator).
*   @note 2: It is recommended to configure TX start time > -5us or
*            -(idleTimeConst-1.28us-DfeLagTime) based on PS apply time.
*/
/* DesignId : MMWL_DesignId_045 */
/* Requirements : AUTORADAR_REQ-798 */
rlReturnVal_t rlRfSetPhaseShiftConfig(rlUInt8_t deviceMap, rlUInt16_t cnt,
                                      rlRfPhaseShiftCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetPhaseShiftConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data) || (cnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};
        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlRfPhaseShiftCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlRfPhaseShiftCfg_t) + (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (cnt <= maxSbCntInMsg)
        {
            sbCntInMsg = cnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = cnt / maxSbCntInMsg;
            lastSbCntInMsg = cnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);
        /* get the loop count in form of number of msg chunks */
        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_SET_MSG,
                                                            RL_RF_PERCHIRPPHASESHIFT_CONF_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlRfPhaseShiftCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(data);
                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to \
                  next chirp Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
            LDRA Tool Issue" */
            /*LDRA_INSPECTED 105 D */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            /* decrement the loop count */
            loopCnt--;
            /* if all phase shift configs have been sent to device then terminate the loop */
            if (loopCnt == 0U)
            {
                break;
            }
        }
    }

    RL_LOGV_ARG0("rlRfSetPhaseShiftConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetPALoopbackConfig(rlUInt8_t deviceMap, rlRfPALoopbackCfg_t* data)
 *
 *   @brief Enable/Disable PA loopback for all enabled profiles
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - PA loopback configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function Enables/Disables PA loopback for all enabled profiles. This is
 *   used for debug purpose that both the TX and RX paths are working properly
 *
 *   @note 1: The PA loop-back configuration API is supported only for debug purpose. Please refer
 *            latest DFP release note for more info.
 */
/* DesignId :  MMWL_DesignId_098 */
/* Requirements : AUTORADAR_REQ-801 */
rlReturnVal_t rlRfSetPALoopbackConfig(rlUInt8_t deviceMap, rlRfPALoopbackCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetPALoopbackConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_PALOOPBACK_CFG_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfPALoopbackCfg_t));
    }

    RL_LOGV_ARG0("rlRfSetPALoopbackConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfSetPSLoopbackConfig(rlUInt8_t deviceMap, rlRfPSLoopbackCfg_t* data)
 *
 *   @brief Enable/Disable Phase shift loopback for all enabled profiles
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Phase shift loopback configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function Enables/Disables Phase shift loopback for all enabled profiles.This is used
 *   to debug the TX (before the PA) and RX chains.
 *
 *   @note 1: The PS loop-back configuration API is supported only for debug purpose. Please refer
 *            latest DFP release note for more info.
 */
/* DesignId : MMWL_DesignId_099 */
/* Requirements : AUTORADAR_REQ-802 */
rlReturnVal_t rlRfSetPSLoopbackConfig(rlUInt8_t deviceMap, rlRfPSLoopbackCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetPSLoopbackConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_PSLOOPBACK_CFG_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfPSLoopbackCfg_t));
    }

    RL_LOGV_ARG0("rlRfSetPSLoopbackConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfSetIFLoopbackConfig(rlUInt8_t deviceMap, rlRfIFLoopbackCfg_t* data)
 *
 *   @brief Enable/Disable RF IF loopback for all enabled profiles.
 *    This is used for debug to check if both TX and RX chains are working correctly.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - IF loopback configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function Enables/Disables RF IF loopback for all enabled profiles. This is used to
 *   debug the  RX IF chain.
 *
 *   @note 1: The IF loop-back configuration API is supported only for debug purpose. Please refer
 *            latest DFP release note for more info.
 */
/* DesignId :  MMWL_DesignId_100 */
/* Requirements : AUTORADAR_REQ-803 */
rlReturnVal_t rlRfSetIFLoopbackConfig(rlUInt8_t deviceMap, rlRfIFLoopbackCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetIFLoopbackConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_MISC_CONF_SET_MSG,
                                       RL_RF_IFLOOPBACK_CFG_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfIFLoopbackCfg_t));
    }

    RL_LOGV_ARG0("rlRfSetIFLoopbackConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfSetProgFiltCoeffRam(rlUInt8_t deviceMap, rlRfProgFiltCoeff_t* data)
 *
 *   @brief Set Programmable Filter coefficient RAM
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - array of coefficients for the programmable filter
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function is used to program the coefficients for the external programmable filter.
 *   This is a new feature added in AWR2243.
 *
 *   @note 1: The programmable filter allow for a trade-off between digital filter chain settling
 *            time and close-in anti-alias attenuation. The Maximum DFE outout sampling rate in
 *            real mode is 25Msps and in complex mode is 22.5Msps. \n
 *            A real-coefficient FIR with up to 63 taps (16-bit coefficients) is supported in both
 *            Complex and real output mode. \n
 *   @note 2: Please refer Table \ref ProgFiltSettings for details on Programmable filter DFE
 *            sampling rate and number of taps. \n
 *   @note 3: This API should be issued before rlSetProfileConfig. \n
 *   @note 4: This API should not be issued when frames are ongoing. \n
 */
/* DesignId :  MMWL_DesignId_050 */
/* Requirements : AUTORADAR_REQ-799 */
rlReturnVal_t rlRfSetProgFiltCoeffRam(rlUInt8_t deviceMap, rlRfProgFiltCoeff_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetProgFiltCoeffRam starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_PROG_FILT_COEFF_RAM_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfProgFiltCoeff_t));
    }

    RL_LOGV_ARG0("rlRfSetProgFiltCoeffRam completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetProgFiltConfig(rlUInt8_t deviceMap, rlRfProgFiltConf_t* data)
 *
 *   @brief Set Programmable Filter configuration
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - programmable filter configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function selects programmable filter cofficient RAM and
 *   map it to configured profile ID.
 *
 *   @note 1: This API is applicable only in xWR1642/xWR6843/xWR1843/AWR2243.
 *   @note 2: This API should not be issued when frames are ongoing.
 */
/* DesignId : MMWL_DesignId_051 */
/* Requirements : AUTORADAR_REQ-800 */
rlReturnVal_t rlRfSetProgFiltConfig(rlUInt8_t deviceMap, rlRfProgFiltConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetProgFiltConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_PROG_FILT_CONF_SET_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfProgFiltConf_t));
    }

    RL_LOGV_ARG0("rlRfSetProgFiltConfig completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetMiscConfig(rlUInt8_t deviceMap, rlRfMiscConf_t* data)
 *
 *   @brief Sets misc feature such as per chirp phase shifter.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Misc configuration such as per chirp phase shifter
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function enables misc feature such as per chirp phase shifter.This API
 *   is valid for devices for which phase shifter is enabled(AWR1243P, AWR2243, AWR1843).
 *
 *   @note : Issue this API first in the sequence if \ref rlRfSetPhaseShiftConfig,
 *   \ref rlSetDynPerChirpPhShifterCfg and \ref rlSetAdvChirpConfig are issued down in the sequence.
 */
/* DesignId :  MMWL_DesignId_052 */
/* Requirements : AUTORADAR_REQ-891 */
rlReturnVal_t rlRfSetMiscConfig(rlUInt8_t deviceMap, rlRfMiscConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetMiscConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_RADAR_MISC_CTL_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfMiscConf_t));
    }

    RL_LOGV_ARG0("rlRfSetMiscConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfSetCalMonTimeUnitConfig(rlUInt8_t deviceMap,
                                                  rlRfCalMonTimeUntConf_t* data)
*
*   @brief Set Calibration monitoring time unit
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - RF Calib Monitoring Time unit config
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function configures calibration monitoring time unit
*
*   @note 1: The Minimum total blank time in a calibMonTimeUnit shall be 1ms to run internal APLL
*           and SYNTH calibrations + ~12.5% of calibMonTimeUnit for  WDT clearing time if WDT is
*           enabled. \n
*   @note 2: Refer to \ref AnalogMonitoringDuration, \ref DigitalMonitoringDuration for
*            the duration of run time monitors and \ref SoftwareOverheads for software overheads.
*   @note 3: The CALIB_MON_TIME_UNIT is applicable for one frame trigger API. Once frame is
*            stopped then FTTI will reset, CALIB_MON_TIME_UNIT is not applicable across
*            multiple SW frame trigger API.
*/
/* DesignId :  MMWL_DesignId_053 */
/* Requirements : AUTORADAR_REQ-892 */
rlReturnVal_t rlRfSetCalMonTimeUnitConfig(rlUInt8_t deviceMap,
                                          rlRfCalMonTimeUntConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetCalMonTimeUnitConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_CALIB_MON_TIME_UNIT_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfCalMonTimeUntConf_t));
    }

    RL_LOGV_ARG0("rlRfSetCalMonTimeUnitConfig completes...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfSetCalMonFreqLimitConfig(rlUInt8_t deviceMap,
                                                    rlRfCalMonFreqLimitConf_t* data)
*
*   @brief Set Calibration monitoring Frequency Limit
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - RF Calib Frequency Limit config
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This function configures limits on RF frequency transmission during calibration
*   and monitoring
*
*   @note 1 : The minimum RF bandwidth shall be set to 200MHz, this is to perform internal
*             calibration and monitoring \n
*   @note 2 : The limit set in this API is not applicable for functional chirps and loop-back
*             chirps used in advanced frame config API. \n
*   @note 3 : The TX0 frequency limit is used by default in calibrations and monitors where TX is
*             not relevant or enabled. \n
*   @note 4 : The RF band used in functional chirp profiles shall be within the limit set in this
*             API. \n
*/
/* DesignId :  MMWL_DesignId_054 */
/* Requirements : AUTORADAR_REQ-893 */
rlReturnVal_t rlRfSetCalMonFreqLimitConfig(rlUInt8_t deviceMap,
                                           rlRfCalMonFreqLimitConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfSetCalMonFreqLimitConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_CAL_MON_FREQ_LIMIT_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfCalMonFreqLimitConf_t));
    }

    RL_LOGV_ARG0("rlRfSetCalMonFreqLimitConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfInitCalibConfig(rlUInt8_t deviceMap, rlRfInitCalConf_t* data)
 *
 *   @brief Set RF Init Calibration Mask bits and report type
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - RF Init calib config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This function configures RF Init calibration mask bits and report type.
 *   Normally, upon receiving rlRfInit API, the Radar SS performs all relevant initial calibrations.
 *   This step can be disabled by setting the corresponding bit in \ref rlRfInitCalConf_t
 *   field to 0x0.If disabled, the host needs to send the calibration data using
 *   \ref rlRfCalibDataRestore so that the RadarSS can operate using the injected calibration data
 *
 *   @note 1 : Each of these calibrations can be selectively disabled by issuing this
 *             message before rlRfInit API.
 *   @note 2 : The APLL, SYNTH1 and SYNTH2 calibrations are always triggred by default on RF init
 *             command.
 */
/* DesignId :  MMWL_DesignId_055 */
/* Requirements : AUTORADAR_REQ-894 */
rlReturnVal_t rlRfInitCalibConfig(rlUInt8_t deviceMap, rlRfInitCalConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfInitCalibConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_INIT_CALIB_CONF_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfInitCalConf_t));
    }

    RL_LOGV_ARG0("rlRfInitCalibConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRfRunTimeCalibConfig(rlUInt8_t deviceMap, rlRunTimeCalibConf_t* data)
 *
 *   @brief Set RF one time & periodic calibration of various RF/analog aspects and trigger
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Runtime calibration config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *  This function configures RF one time & periodic calibration of various RF/analog aspects
 *  and trigger. The response is in the form of an asynchronous event. The calibration would
 *  be performed by Radar SS during while framing during inter-burst idle time slot of 250uS.
 *
 *  @note 1: This API must be called after rlSetProfileConfig
 *  @note 2: This API should be issued when the device is not framing.
 *  @note 3: This API can be issued when the device is framing, the calibration periodicity update
 *           or one time calibrations can be done while frames are running.
 */
/* DesignId : MMWL_DesignId_056 */
/* Requirements : AUTORADAR_REQ-895, AUTORADAR_REQ-1063 */
rlReturnVal_t rlRfRunTimeCalibConfig(rlUInt8_t deviceMap, rlRunTimeCalibConf_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfRunTimeCalibConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_RUN_TIME_CALIB_CONF_TRIG_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRunTimeCalibConf_t));
    }

    RL_LOGV_ARG0("rlRfRunTimeCalibConfig completes...\n");
    return retVal;
}

/** @fn rlReturnVal_t rlRxGainTempLutSet(rlUInt8_t deviceMap, rlRxGainTempLutData_t *data)
 *
 *   @brief Overwrite RX gain temperature Lookup Table(LUT) in Radar SS
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - RX gain Temperature LUT config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API can be used to overwrite the RX gain Lookup Table(LUT) for different temperature
 *   used in RadarSS.
 *
 *   @note 1 : This API should be issued after profile configuration API.
 */
/* DesignId : MMWL_DesignId_057 */
/* Requirements : AUTORADAR_REQ-896 */
rlReturnVal_t rlRxGainTempLutSet(rlUInt8_t deviceMap, rlRxGainTempLutData_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_RX_GAIN_TEMPLUT_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRxGainTempLutData_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRxGainTempLutGet(rlUInt8_t deviceMap, rlRxGainTempLutRead_t* inData,
                                                               rlRxGainTempLutInject_t *outData)
*
*   @brief Gets RX gain temperature Lookup Table(LUT) in Radar SS
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] inData - RX gain Temperature LUT request  config
*   @param[out] outData - RX gain Temperature LUT read config
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API is to read the temperature based RX gain LUT used by the Radar SS.
*   This API should be issued after the profile configuration API.
*/
/* DesignId : MMWL_DesignId_058 */
/* Requirements : AUTORADAR_REQ-897 */
rlReturnVal_t rlRxGainTempLutGet(rlUInt8_t deviceMap, rlRxGainTempLutReadReq_t *inData,
                                 rlRxGainTempLutData_t *outData)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRxGainTempLutRead starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == inData))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlRxGainTempLutRead, Invalid device map\n");
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};

        /* Fill in-message Payload */
        rlDriverFillPayload(RL_RF_DYNAMIC_CONF_GET_MSG, RL_RF_RX_GAIN_TEMPLUT_CONF_SB,
                            &inPayloadSb, (rlUInt8_t *)inData,
                            (rlUInt16_t)sizeof(rlRxGainTempLutReadReq_t));

        /* Construct command packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_GET_MSG, &inMsg, &inPayloadSb);

        /* Fill out-message Payload */
        rlDriverFillPayload(0U, 0U, &outPayloadSb, (rlUInt8_t *)outData,
                            (rlUInt16_t)sizeof(rlRxGainTempLutData_t));

        /* Construct response packet */
        rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);

        /* Send Command to mmWave Radar Device */
        retVal = rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
    }
    RL_LOGV_ARG0("rlRxGainTempLutRead ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlTxGainTempLutSet(rlUInt8_t deviceMap, rlTxGainTempLutData_t *data)
 *
 *   @brief Overwrites TX gain temperature based Lookup table (LUT)
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - TX gain Temperature LUT config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API can be used to overwrite the TX gain temperature LUT used in Radar SS.
 *   This API should be issued after profile configuration API.
 */
/* DesignId : MMWL_DesignId_059 */
/* Requirements : AUTORADAR_REQ-898 */
rlReturnVal_t rlTxGainTempLutSet(rlUInt8_t deviceMap, rlTxGainTempLutData_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_TX_GAIN_TEMPLUT_CONF_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlTxGainTempLutData_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRxGainTempLutGet(rlUInt8_t deviceMap, rlTxGainTempLutRead_t* inData,
                                                               rlTxGainTempLutInject_t *outData)
*
*   @brief Gets TX gain temperature Lookup table (LUT)
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] inData - TX gain Temperature LUT request config
*   @param[out] outData - TX gain Temperature LUT read config
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API is to read the temperature based TX gain LUT used by the firmware.
*   This API should be issued after the rlSetProfileConfig API.
*/
/* DesignId : MMWL_DesignId_060 */
/* Requirements : AUTORADAR_REQ-899 */
rlReturnVal_t rlTxGainTempLutGet(rlUInt8_t deviceMap,
                                 rlTxGainTempLutReadReq_t *inData, rlTxGainTempLutData_t *outData)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlTxGainTempLutRead starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == inData))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlTxGainTempLutRead, Invalid device map\n");
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};

        /* Fill in-message Payload */
        rlDriverFillPayload(RL_RF_DYNAMIC_CONF_GET_MSG, RL_RF_TX_GAIN_TEMPLUT_CONF_SB,
                            &inPayloadSb, (rlUInt8_t *)inData,
                            (rlUInt16_t)sizeof(rlTxGainTempLutReadReq_t));

        /* Construct command packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_GET_MSG, &inMsg, &inPayloadSb);

        /* Fill out-message Payload */
        rlDriverFillPayload(0U, 0U, &outPayloadSb, (rlUInt8_t *)outData,
                            (rlUInt16_t)sizeof(rlTxGainTempLutData_t));

        /* Construct response packet */
        rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);

        /* Send Command to mmWave Radar Device */
        retVal = rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
    }
    RL_LOGV_ARG0("rlTxGainTempLutRead ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfTxFreqPwrLimitConfig(rlUInt8_t deviceMap,
                                               rlRfTxFreqPwrLimitMonConf_t* data)
*
*   @brief Sets the limits for RF frequency transmission for each TX and also TX power limits
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - Tx Rf freq and power limit config data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API sets the limits for RF frequency transmission for each TX and also TX power limits.
*
*   @note 1 : The minimum RF bandwidth shall be set to 200MHz, this is to perform internal
*             calibration and monitoring \n
*   @note 2 : The limit set in this API is not applicable for functional chirps and loop-back
*             chirps used in advanced frame config API. \n
*   @note 3 : The TX0 frequency limit is used by default in calibrations and monitors where TX is
*             not relevant or enabled. \n
*   @note 4 : The RF band used in functional chirp profiles shall be within the limit set in this
*             API. \n
*/
/* DesignId : MMWL_DesignId_066 */
/* Requirements : AUTORADAR_REQ-905 */
rlReturnVal_t rlRfTxFreqPwrLimitConfig(rlUInt8_t deviceMap, rlRfTxFreqPwrLimitMonConf_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_TX_FREQ_PWR_LIMIT_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfTxFreqPwrLimitMonConf_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetLoopBckBurstCfg(rlUInt8_t deviceMap, rlLoopbackBurst_t *data)
 *
 *   @brief This API is used to introduce loopback chirps within the functional frames.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Loopback chirp config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API can be used to introduce loopback chirps within the functional frames. This loopback
 *   chirps will be introduced only if advanced frame configuration is used where user can define
 *   which sub-frame contains loopback chirps. The following loopback configuration will apply to
 *   one burst and user can program up to 16 different loopback configurations in 16 different
 *   bursts of a given sub-frame. User has to ensure that the corresponding sub-frame is defined in
 *   rlSetAdvFrameConfig and sufficient time is given to allow the loopback bursts to be
 *   transmitted.
 *
 *   @note 1: If user desires to enable loopback chirps within functional frames, then this API
 *            should be issued before rlSetProfileConfig
 *   @note 2: Only profile based phase shifter is supported in loopback configuration. Per-chirp
 *            phase shifter if enabled will not be reflected in loopback chirps.
 *   @note 3: For the sub-frame in which loopback is desired, user should set numOfChirps per burst
 *            as 1 and can use numLoops per burst for multiple chirps in the burst.
 */
/* DesignId : MMWL_DesignId_062 */
/* Requirements : AUTORADAR_REQ-901 */
rlReturnVal_t rlSetLoopBckBurstCfg(rlUInt8_t deviceMap, rlLoopbackBurst_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_LB_BURST_CFG_SET_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlLoopbackBurst_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetDynChirpCfg(rlUInt8_t deviceMap, rlUInt16_t segCnt,
                                          rlDynChirpCfg_t **data)
*
*   @brief Injects chirp configuration to be programmed dynamically
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] segCnt - number of segments for which application sends array of data.
*   @param[in] data - Dynamic chirp configuration
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API can be used to dynamically change the chirp configuration while frames are on-going.
*   The configuration will be stored in software and at rlDynChirpEnCfg API invocation radarSS
*   copies these chirp configurations from SW RAM to HW RAM at the end of current on-going frame.
*
*   @note : The new feature of dynamic chirp configuaration to configuare 48 chirps in one API
*           is not applicable in xWR6843.
*/
/* DesignId : MMWL_DesignId_063 */
/* Requirements : AUTORADAR_REQ-902 */
rlReturnVal_t rlSetDynChirpCfg(rlUInt8_t deviceMap, rlUInt16_t segCnt,
                               rlDynChirpCfg_t **data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data) ||
        (segCnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};

        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlDynChirpCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlDynChirpCfg_t) + (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (segCnt <= maxSbCntInMsg)
        {
            sbCntInMsg = segCnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = segCnt / maxSbCntInMsg;
            lastSbCntInMsg = segCnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);

        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_SET_MSG,
                                                            RL_RF_DYN_CHIRP_CFG_SET_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlDynChirpCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(*data);

                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to \
                 next Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            loopCnt--;

            if (loopCnt == 0U)
            {
                break;
            }
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetDynChirpEn(rlUInt8_t deviceMap, rlDynChirpEnCfg_t *data)
 *
 *   @brief Triggers copy of chirp config from SW to HW RAM.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Dynamic chirp enable configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API can be used to trigger the copy of chirp configuration from software to hardware RAM.
 *   The copy will be performed at the end of the ongoing frame active window (start of the frame
 *   idle time). \n
 *   @note 1: User needs to invoke this API within inter-frame idle time, not at boundary of frame
 *            end. Since dynamic chirps are configured at run time, there is not error checks done
 *            on the input data. If input data is out of range or invalid, device might misbehave.
 *   @note 2: HW reconfiguration time is around 500 us. User has to ensure that
 *            AWR_DYN_CHIRP_ENABLE_SB API is issued at least 500 us before the end of the ongoing
 *            frame active window (start of the frame idle time) to apply configurations for next
 *            frame onwards. \n
 */
/* DesignId : MMWL_DesignId_064 */
/* Requirements : AUTORADAR_REQ-903 */
rlReturnVal_t rlSetDynChirpEn(rlUInt8_t deviceMap, rlDynChirpEnCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_DYN_CHIRP_CFG_EN_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlDynChirpEnCfg_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetDynPerChirpPhShifterCfg(rlUInt8_t deviceMap, rlUInt16_t segCnt,
                                                   rlDynPerChirpPhShftCfg_t **data)
*
*   @brief Injects per-chirp phase shifter configuration to be applied dynamically
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] segCnt - number of segments for which application sends array of data.
*   @param[in] data - Dynamic chirp configuration
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API can be used to dynamically change the per-chirp phase shifter configuration while
*   frames are on-going. The configuration will be stored in software and the new configuration
*   will be applied at the end of the on-going frame. Note that the configuration should be
*   received by the firmware 10 ms before the end of the current frame.

*   @note 1: This API is valid only for AWR1243P/2243 mmWave device when mmWaveLink instance is
*            running on External Host Processor.
*   @note 2: Phase shifter(PS) settings are applied in advance at max -5us or at
*            -(idleTimeConst-1.28us-DfeLagTime) from the knee of the ramp.
*            If idle time > 6.28us, then PS is applied always at -5us and
*            if idle time < 6.28us, then PS is applied at -(idleTimeConst-1.28us-DfeLagTime) from
*            knee of the ramp.
*            Where DfeLagTime is internal DFE lag time (Please refer rampgen calculator).
*   @note 3: It is recommended to configure TX start time > -5us or
*            -(idleTimeConst-1.28us-DfeLagTime) based on PS apply time.
*/
/* DesignId : MMWL_DesignId_065 */
/* Requirements : AUTORADAR_REQ-904 */
rlReturnVal_t rlSetDynPerChirpPhShifterCfg(rlUInt8_t deviceMap, rlUInt16_t segCnt,
                                           rlDynPerChirpPhShftCfg_t **data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data) ||
        (segCnt == 0U))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg;
        rlDriverMsg_t outMsg = {0};

        /* Index and other paramerters to send multiple sub blocks in one/more commands chunk */
        rlUInt16_t sbLen, maxSbCntInMsg, sbCntInMsg, numChnkOfMsg, lastSbCntInMsg, loopCnt, indx;

        /* Initialize Command and Response Sub Blocks */
        rlPayloadSb_t inPayloadSb[RL_CMD_PL_LEN_MAX / (sizeof(rlDynPerChirpPhShftCfg_t) +
                                                       RL_SBC_ID_SIZE + RL_SBC_LEN_SIZE)];

        /* single sub-block length for chirp config API */
        sbLen = (rlUInt16_t)(sizeof(rlDynPerChirpPhShftCfg_t) +
                             (RL_SBC_LEN_SIZE + RL_SBC_ID_SIZE));

        /* get Max Sub Block count */
        /* AR_CODE_REVIEW MR:D.4.1 <APPROVED> "sbLen can not be zero" */
        /*LDRA_INSPECTED 127 D */
        maxSbCntInMsg = (RL_CMD_PL_LEN_MAX / sbLen);

        retVal = RL_RET_CODE_OK;
        /* if requested count of chirpConfig is within one Message packet */
        if (segCnt <= maxSbCntInMsg)
        {
            sbCntInMsg = segCnt;
            numChnkOfMsg = 1U;
            lastSbCntInMsg = 0U;
        }
        else
        {
            sbCntInMsg = maxSbCntInMsg;
            numChnkOfMsg = segCnt / maxSbCntInMsg;
            lastSbCntInMsg = segCnt % maxSbCntInMsg;
        }

        /* Fill in-message packet */
        rlDriverConstructInMsg(RL_RF_DYNAMIC_CONF_SET_MSG, &inMsg, &inPayloadSb[0]);

        loopCnt = (lastSbCntInMsg == 0U) ? numChnkOfMsg : (numChnkOfMsg + 1U);

        while (retVal == RL_RET_CODE_OK)
        {
            /* all full messages have been sent, then send last partial message */
            if ((loopCnt == 1U) && (lastSbCntInMsg != 0U))
            {
                inMsg.opcode.nsbc = lastSbCntInMsg;
            }
            else
            {
                inMsg.opcode.nsbc = sbCntInMsg;
            }

            for (indx = 0U; indx < inMsg.opcode.nsbc; indx++)
            {
                /* Set Command Sub Block*/
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function.\
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].sbid = RL_GET_UNIQUE_SBID(RL_RF_DYNAMIC_CONF_SET_MSG,
                                                            RL_RF_DYN_PERCHIRP_PHSHFT_CFG_SET_SB);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].len = (rlUInt16_t)sizeof(rlDynPerChirpPhShftCfg_t);
                /* AR_CODE_REVIEW MR:R.2.2  <APPROVED> "Values are used by called function. \
                LDRA Tool Issue" */
                /*LDRA_INSPECTED 105 D */
                inPayloadSb[indx].pSblkData = (rlUInt8_t *)(*data);
                /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to \
                next Config */
                /*LDRA_INSPECTED 567 S */
                data++;
            }
            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            loopCnt--;

            if (loopCnt == 0U)
            {
                break;
            }
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRfCalibDataRestore(rlUInt8_t deviceMap, rlCalibrationData_t *data)
 *
 *   @brief Injects calibration data to the device
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Calibration data of 3 chunks stored at application space
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API restores the calibration data which was stored previously using the
 *   rlCalibDataStore command. Application needs to feed in 3 chunks of calibration data.
 *   recommended API sequence for calibration data restore is: \n
 *   1. rlRfCalibDataRestore (To restore factory calibration data to avoid on field
 *      RF interference during calibration) \n
 *   2. Wait for RL_RF_AE_INITCALIBSTATUS_SB event. \n
 *   3. rlRfInitCalibConfig (Enable only required calibration to run) \n
 *   4. rlRfInit: This triggers very basic calibrations and RF initializations. \n
 *   5. Wait for RL_RF_AE_INITCALIBSTATUS_SB event. \n
 *
 *   @note 1: Once the calibration data is restored properly in radarSS SW RAM and validated,
 *            mmWave Front end would send asynchronous event RL_RF_AE_INITCALIBSTATUS_SB indicating
 *            the result of the calibrations based on Calib data sent by the application, this
 *            indicates success of the calibration data restore.
 *   @note 2: All 3 chunks of 224 bytes each shall be sent to radar device to complete the restore
 *            process and to generate RL_RF_AE_INITCALIBSTATUS_SB.
 */
/* DesignId : MMWL_DesignId_067 */
/* Requirements : AUTORADAR_REQ-906 */
rlReturnVal_t rlRfCalibDataRestore(rlUInt8_t deviceMap, rlCalibrationData_t *data)
{
    rlReturnVal_t retVal;
    rlUInt8_t idx;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};
        /* set return value to zero by default */
        retVal = RL_RET_CODE_OK;
        /* Invoke Set Calibration data command for RL_MAX_CALIB_DATA_CHUNK chunks */
        for (idx = 0U; idx < RL_MAX_CALIB_DATA_CHUNK; idx++)
        {
            /* Construct command packet */
            rlDriverConstructInMsg(RL_RF_STATIC_CONF_SET_MSG, &inMsg, &inPayloadSb);
            /* Fill in-message Payload */
            /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
              next calibData */
            /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
                          next calibData */
            /*LDRA_INSPECTED 87 S */
            /*LDRA_INSPECTED 567 S */
            rlDriverFillPayload(RL_RF_STATIC_CONF_SET_MSG, RL_RF_CAL_DATA_RD_WR_SB, &inPayloadSb,
                                (rlUInt8_t *)(&data->calibChunk[idx]),
                                (rlUInt16_t)sizeof(rlCalDataStore_t));

            /* Construct response packet */
            rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
            /* Fill in-message Payload */
            rlDriverFillPayload(0U, 0U, &outPayloadSb, NULL, 0U);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            /* check for return value */
            if (RL_RET_CODE_OK != retVal)
            {
                /* If command invoke is failed then terminate this loop */
                break;
            }
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRfCalibDataStore(rlUInt8_t deviceMap, rlCalibrationData_t *data)
 *
 *   @brief Read calibration data from the device
 *   @param[in] deviceMap - Bitmap of devices to send the message.
 *   @param[in] data - Calibration data of 3 chunks which will filled by device.
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API reads the calibration data from the device which can be injected later using the
 *   rlCalibDataRestore command. RadarSS will return 3 chunks of calibration data.
 *
 *   @note 1: The total size of the calibration data is 672 bytes, this has been split into
 *            3 chunks (numOfChunk)of 224 bytes each due to SPI limitation. The Host should
 *            receive all these 3 chunks from radar device, later host can store only relevant
 *            data in non volatile memory.
 *   @note 2: Before storing the calibration data in non volatile memory, the host shall make sure
 *            validity status of all enabled calibrations are SET to value 1 including APLL, VCO1,
 *            VCO2 and LODIST calibration validity in rlRfInit of radar device.
 *   @note 3: Host can store only relevant calibration data in non volatile memory and
 *            corresponding validity bits shall be set to 1 in rlRfCalibDataRestore and rest
 *            of the validity bits should be clear to 0 before restoring the data to radar device.
 *   @note 4: Host shall ignore APLL, VCO1, VCO2 and LODIST calibration validity bits while
 *            restoring, these calibrations will be done in each device power-up.
 */
/* DesignId : MMWL_DesignId_068 */
/* Requirements : AUTORADAR_REQ-906 */
rlReturnVal_t rlRfCalibDataStore(rlUInt8_t deviceMap, rlCalibrationData_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};
        rlCalDataGetCfg_t caldataGetCfg = {0U};
        rlUInt8_t idx;
        /* set return value to zero by default */
        retVal = RL_RET_CODE_OK;
        /* Invoke Get Calibration data command for RL_MAX_CALIB_DATA_CHUNK chunks */
        for (idx = 0U; idx < RL_MAX_CALIB_DATA_CHUNK; idx++)
        {
            /* Construct command packet */
            rlDriverConstructInMsg(RL_RF_STATIC_CONF_GET_MSG, &inMsg, &inPayloadSb);
            /* Fill in-message Payload */
            rlDriverFillPayload(RL_RF_STATIC_CONF_GET_MSG, RL_RF_CAL_DATA_RD_WR_SB, &inPayloadSb,
                                (rlUInt8_t *)&caldataGetCfg, (rlUInt16_t)sizeof(rlCalDataGetCfg_t));

            /* Construct response packet */
            rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
            /* Fill in-message Payload */
            /*AR_CODE_REVIEW MR:R.18.1 <APPROVED> "require pointer increment to jump to
              next calibData */
            /*AR_CODE_REVIEW MR:R.18.4 <APPROVED> "require pointer increment to jump to
              next calibData */
            /*LDRA_INSPECTED 87 S */
            /*LDRA_INSPECTED 567 S */
            rlDriverFillPayload(0U, 0U, &outPayloadSb, (rlUInt8_t *)(&data->calibChunk[idx]), 0U);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            /* check for return value */
            if (RL_RET_CODE_OK != retVal)
            {
                /* If command invoke is failed then terminate this loop */
                break;
            }
            /* increment Chunk ID for next Get Command */
            caldataGetCfg.chunkId++;
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRfInterRxGainPhaseConfig(rlUInt8_t deviceMap, rlInterRxGainPhConf_t* data)
 *
 *   @brief Sets different Rx gain/phase offset
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Inter RX gain, phase offset config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API can be used to compensate various RX and gain/phase offsets and same API can be used
 *   to estimation the same using TX frequency shift.
 *
 *   @note 1: This API is supported only on AWR2243 device. Please refer latest DFP release note
 *           for more info. \n
 *   @note 2: Issue this API first in the sequence before \ref rlSetProfileConfig API.
 *   @note 3: The Digital TX frequency shift enable mode in below API is for debug purpose only,
 *           the functional phase shifter will not be operational when this mode is used. It is
 *           recommended to re-issue profile config API after disabling this mode before running
 *           functional frames. \n
 *   @note 4: An application for the digital delay compensation field may be to digitally
 *           compensate for linear IF frequency dependent phase mismatch (e.g. mismatch across
 *           devices in cascaded operation, caused by DIG_SYNC_IN path mismatches). The residual
 *           phase error can be up to +/-0.5 degree. \n
 */
/* DesignId : MMWL_DesignId_061 */
/* Requirements : AUTORADAR_REQ-900 */
rlReturnVal_t rlRfInterRxGainPhaseConfig(rlUInt8_t deviceMap,
                                         rlInterRxGainPhConf_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_DIG_COMP_EST_CTRL_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlInterRxGainPhConf_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlGetRfBootupStatus(rlUInt8_t deviceMap, rlRfBootStatusCfg_t *data)
 *
 *   @brief Get radarSS bootup status
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - bootup status configuration
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API gets the radarSS bootup status
 */
/* DesignId :  MMWL_DesignId_096 */
/* Requirements : AUTORADAR_REQ-911 */
rlReturnVal_t rlGetRfBootupStatus(rlUInt8_t deviceMap, rlRfBootStatusCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check for NULL pointer */
    if (data == RL_NULL_PTR)
    {
        /* set error code if data pointer is passed NULL */
        retVal = RL_RET_CODE_NULL_PTR;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_STATUS_GET_MSG,
                                       RL_RF_BOOTUP_BIST_STATUS_SB, (rlUInt8_t *)data, 0U);
    }

    return retVal;
}

/** @fn rlReturnVal_t rlSetInterChirpBlkCtrl(rlUInt8_t deviceMap, rlInterChirpBlkCtrlCfg_t *data)
 *
 *   @brief Sets Inter-chip turn on and turn off times or various RF blocks
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Inter chirp block control config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API programs the Inter-chip turn on and turn off times or various RF blocks
 *   @note The minimum inter-chirp time should be greater than maximum of the following
 *   1. abs(rx02RfTurnOffTime) + max(abs(rx02RfPreEnTime), abs(rx02RfTurnOnTime))
 *   2. abs(rx13RfTurnOffTime) + max(abs(rx13RfPreEnTime), abs(rx13RfTurnOnTime))
 *   3. abs(rx02BbTurnOffTime) + max(abs(rx02BbPreEnTime), abs(rx02BbTurnOnTime))
 *   4. abs(rx13BbTurnOffTime) + max(abs(rx13BbPreEnTime), abs(rx13BbTurnOnTime))
 *   5. abs(rxLoChainTurnOffTime) + abs(rxLoChainTurnOnTime)
 *   6. abs(txLoChainTurnOffTime) + abs(txLoChainTurnOnTime)
 *
 *   @note : The inter-chirp timing control configuration API is supported in this release. Please
 *           refer latest DFP release note for more info.
 */
/* DesignId :  MMWL_DesignId_097 */
/* Requirements : AUTORADAR_REQ-912 */
rlReturnVal_t rlSetInterChirpBlkCtrl(rlUInt8_t deviceMap,
                                     rlInterChirpBlkCtrlCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_INTERCHIRP_BLOCK_CTRL_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlInterChirpBlkCtrlCfg_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetSubFrameStart(rlUInt8_t deviceMap, rlSubFrameStartCfg_t *data)
 *
 *   @brief Triggers the next sub-frame in software triggered sub-frame mode
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Sub-frame start config
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API triggers the next sub-frame in software triggered sub-frame mode
 *
 *   @note 1: If the user wishes to trigger each sub-frame independently, then after advanced frame
 *            config, the rlSensorStart should be issued once using rlSensorStop. This does not
 *            start any sub-frames but it will prepare the hardware for sub-frame trigger. Next any
 *            subsequent sub-frame trigger will start the sub-frames.
 *   @note 2: If the user wishes to use sub-frame trigger, he has to ensure that sub-frame trigger
 *            command is issued k*N times where k is the number of sub-frames in each frame and N is
 *            the number of frames. If the user wishes to stop frames in between, then he has to
 *            issue the rlSensorStop only after k*M triggers of sub-frame trigger command (where M
 *            is an integer). i.e. rlSensorStop can be issued only at frame boundaries.
 *   @note 3: If software based sub-frame trigger mode is chosen by the user, watchdog feature will
 *            not be available. User has to ensure that the watchdog is disabled before enabling the
 *            software based sub-frame trigger mode.
 *   @note 4: If sub-frame trigger or hardware trigger mode is used to trigger the frames/sub-
 *            frames and if frames need to be stopped before the specified number of frames, then
 *            the the frame stop command using rlSensorStop API should be issued
 *            while the frame is on-going. If the frames are stopped while the device is idle,
 *            it can lead to errors
 */
/* DesignId :  MMWL_DesignId_107 */
/* Requirements : AUTORADAR_REQ-938, AUTORADAR_REQ-1021 */
rlReturnVal_t rlSetSubFrameStart(rlUInt8_t deviceMap,
                                 rlSubFrameStartCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_SUBFRAME_START_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlSubFrameStartCfg_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRfPhShiftCalibDataRestore(rlUInt8_t deviceMap,
 *                                                 rlPhShiftCalibrationData_t *data)
 *
 *   @brief Injects phase shifter calibration data to the device
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - Calibration data of number of TX channels enabled chunks stored at
 *                     application space
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API restores the phase shifter calibration data which was stored previously using the
 *   rlRfPhShiftCalibDataStore command. Application needs to feed number of TX channels enabled
 *   chunks of phase shifter calibration data. This is device specific feature, please refer data
 *   sheet.
 */
/* DesignId : MMWL_DesignId_109 */
/* Requirements : AUTORADAR_REQ-1003 */
rlReturnVal_t rlRfPhShiftCalibDataRestore(rlUInt8_t deviceMap, rlPhShiftCalibrationData_t *data)
{
    rlReturnVal_t retVal;
    rlUInt8_t idx;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};
        /* set return value to zero by default */
        retVal = RL_RET_CODE_OK;
        /* Invoke Set Calibration data command for RL_MAX_CALIB_DATA_CHUNK chunks */
        for (idx = 0U; idx < RL_TX_CNT; idx++)
        {
            /* Construct command packet */
            rlDriverConstructInMsg(RL_RF_STATIC_CONF_SET_MSG, &inMsg, &inPayloadSb);
            if (idx == (RL_TX_CNT - 1U))
            {
                data->PhShiftcalibChunk[idx].calibApply = 1U;
            }
            /* Fill in-message Payload */
            /*AR_CODE_REVIEW MR:R.18.1 <REVIEWED> "require pointer increment to jump to
            next calibData */
            /*AR_CODE_REVIEW MR:R.18.1 <REVIEWED> "require pointer increment to jump to
            next calibData */
            /*LDRA_INSPECTED 87 S */
            /*LDRA_INSPECTED 567 S */
            rlDriverFillPayload(RL_RF_STATIC_CONF_SET_MSG, RL_RF_PH_SHIFT_CAL_DATA_RD_WR_SB,
                                &inPayloadSb, (rlUInt8_t *)(&data->PhShiftcalibChunk[idx]),
                                (rlUInt16_t)sizeof(rlPhShiftCalibrationStore_t));

            /* Construct response packet */
            rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
            /* Fill in-message Payload */
            rlDriverFillPayload(0U, 0U, &outPayloadSb, NULL, 0U);

            /* Send Command to mmWave Radar Device */
            retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
            /* check for return value */
            if (RL_RET_CODE_OK != retVal)
            {
                /* If command invoke is failed then terminate this loop */
                break;
            }
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlRfPhShiftCalibDataStore(rlUInt8_t deviceMap,
 *                                                 rlPhShiftCalibrationData_t *data)
 *
 *   @brief Read calibration data from the device
 *   @param[in] deviceMap - Bitmap of devices to send the message.
 *   @param[in] data - Phase shift calibration data of number of TX channels enbled chunks which
 *                     will filled by device.
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API reads the phase shifter calibration data from the device which can be injected later
 *   using the rlRfPhShifterCalibDataRestore command. RadarSS will return number of TX chunks of
 *   phase shifter calibration data.This is device specific feature, please refer data sheet.
 */
/* DesignId : MMWL_DesignId_108 */
/* Requirements : AUTORADAR_REQ-1003 */
rlReturnVal_t rlRfPhShiftCalibDataStore(rlUInt8_t deviceMap, rlPhShiftCalibrationData_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Initialize Command and Response Sub Blocks */
        rlDriverMsg_t inMsg = {0};
        rlDriverMsg_t outMsg = {0};
        rlPayloadSb_t inPayloadSb = {0};
        rlPayloadSb_t outPayloadSb = {0};
        rlPhShiftCalDataGetCfg_t phShifterCaldataGetCfg = {0U};
        rlUInt8_t idx;
        /* set return value to zero by default */
        /* AR_CODE_REVIEW MR:R.2.2 <REVIEWED> "retVal initialized to 0 if not it means error" */
        /*LDRA_INSPECTED 8 D */
        retVal = RL_RET_CODE_OK;
        /* Invoke Get Calibration data command for RL_MAX_CALIB_DATA_CHUNK chunks */
        for (idx = 0U; idx < RL_TX_CNT; idx++)
        {
            if ((RL_NULL_PTR != &data->PhShiftcalibChunk[idx]))
            {
                phShifterCaldataGetCfg.txIndex = data->PhShiftcalibChunk[idx].txIndex;
                /* Construct command packet */
                rlDriverConstructInMsg(RL_RF_STATIC_CONF_GET_MSG, &inMsg, &inPayloadSb);
                /* Fill in-message Payload */
                rlDriverFillPayload(RL_RF_STATIC_CONF_GET_MSG, RL_RF_PH_SHIFT_CAL_DATA_RD_WR_SB,
                                    &inPayloadSb, (rlUInt8_t *)&phShifterCaldataGetCfg,
                                    (rlUInt16_t)sizeof(rlPhShiftCalDataGetCfg_t));

                /* Construct response packet */
                rlDriverConstructOutMsg(1U, &outMsg, &outPayloadSb);
                /* Fill in-message Payload */
                /*AR_CODE_REVIEW MR:R.18.1 <REVIEWED> "require pointer increment to jump to
                  next calibData */
                /*AR_CODE_REVIEW MR:R.18.4 <REVIEWED> "require pointer increment to jump to
                  next calibData */
                /*LDRA_INSPECTED 87 S */
                /*LDRA_INSPECTED 567 S */
                rlDriverFillPayload(0U, 0U, &outPayloadSb,
                                    (rlUInt8_t *)(&data->PhShiftcalibChunk[idx]), 0U);

                /* Send Command to mmWave Radar Device */
                retVal += rlDriverCmdInvoke(deviceMap, inMsg, &outMsg);
                /* check for return value */
                if (RL_RET_CODE_OK != retVal)
                {
                    /* If command invoke is failed then terminate this loop */
                    break;
                }
            }
        }
    }
    return retVal;
}

/** @fn rlReturnVal_t rlGetRfDieId(rlUInt8_t deviceMap, rlRfDieIdCfg_t *data)
 *
 *   @brief Get device die ID status
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[out] data - Die ID status
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API gets the device Die ID status
 */
/* DesignId :  MMWL_DesignId_110 */
/* Requirements : AUTORADAR_REQ-1004, AUTORADAR_REQ-1027 */
rlReturnVal_t rlGetRfDieId(rlUInt8_t deviceMap, rlRfDieIdCfg_t *data)
{
    rlReturnVal_t retVal;

    /* check for NULL pointer */
    if (data == RL_NULL_PTR)
    {
        /* set error code if data pointer is passed NULL */
        retVal = RL_RET_CODE_NULL_PTR;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_STATUS_GET_MSG,
                                       RL_RF_DIEID_STATUS_SB, (rlUInt8_t *)data, 0U);
    }

    return retVal;
}

/** @fn rlReturnVal_t rlRfGetCpuFault(rlUInt8_t deviceMap, rlCpuFault_t *data)
 *
 *   @brief Get RadarSS CPU fault status
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data  - Container for RadarSS CPU fault status
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API gets the RadarSS CPU fault status.
 */
/* DesignId : MMWL_DesignId_123 */
/* Requirements : AUTORADAR_REQ-1039, AUTORADAR_REQ-1060 */
rlReturnVal_t rlRfGetCpuFault(rlUInt8_t deviceMap, rlCpuFault_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfGetCpuFault starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set error code if DeviceMAP is invalid or data pointer is null */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_STATUS_GET_MSG,
                                       RL_RF_RFCPUFAULT_STATUS_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlCpuFault_t));
    }
    RL_LOGV_ARG0("rlRfGetCpuFault ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfGetEsmFault(rlUInt8_t deviceMap, rlBssEsmFault_t *data)
 *
 *   @brief Get RadarSS ESM fault status.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data  - Container for RadarSS ESM fault status
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API gets the RadarSS ESM fault status.
 */
/* DesignId : MMWL_DesignId_123 */
/* Requirements : AUTORADAR_REQ-1039 */
rlReturnVal_t rlRfGetEsmFault(rlUInt8_t deviceMap, rlBssEsmFault_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlRfGetEsmFault starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set error code if DeviceMAP is invalid or data pointer is null */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("Invalid input");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteGetApi(deviceMap, RL_RF_STATUS_GET_MSG,
                                       RL_RF_RFESMFAULT_STATUS_SB, (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlBssEsmFault_t));
    }
    RL_LOGV_ARG0("rlRfGetEsmFault ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetAdvChirpConfig(rlUInt8_t deviceMap, rlAdvChirpCfg_t *data)
 *
 *   @brief Set the Advanced chirp configuration to the device.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - structure contains adv-chirp configs
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API defines the programming of advanced chirp configurations for each chirp parameters to
 *   generate a waveform pattern in a frame/burst. It provides ability to program fixed delta
 *   increment (Delta dither) for certain chirp parameters (eg. chirp start frequency, idle time,
 *   phase shifter), on top of unique dithers selected from configurable look-up-table (LUT Dither).
 *   The configurable look-up-table is an array of values loaded into a pre-configured Generic SW
 *   Chirp Parameter LUT. The size of the generic LUT is 12kB and user has the flexibility to
 *   program any number of unique dithers for each chirp parameters. Thus the user can achieve
 *   fixed increment, or LUT based dither, or a combination of both. \n
 *
 *   When using the Advanced Chirp Config API, there are some implications to frame config and
 *   advanced frame config APIs. Specifically, the CHIRP_START_INDX and CHIRP_END_INDX fields are
 *   no longer applicable, and the NUM_LOOPS field has a different meaning in the sense that this
 *   field now denotes the total number of chirps in the frame/burst. Please refer
 *   AWR_FRAME_CONF_SET_SB and AWR_ADVANCED_FRAME_CONF_SB APIs with the updated field descriptions
 *   as below. \n
 *
 *   The total number of chirps L in a burst should be programmed as per below calculation in frame
 *   configuration API (using the NUM_LOOPS field). \n
 *   L = X * Y, where X is 1 to 512 (supported HW CHIRP RAM) and Y is 1 to 128 (supported HW CHIRP
 *   LOOPS) \n
 *   The value of L should be a multiple of 4 (assuming each chirp is min 25us duration) i.e 1, 4,
 *   8, 12, 16, 20, 24 ¦ 32768 (max). The FW needs to prepare and update HW CHIRP RAM dynamically
 *   in advance chirp config API, this puts some restriction on minimum number of chirps in a
 *   burst/frame. \n
 *
 *   Using this API, four types of control can be achieved on each parameters of a chirp. \n
 *   1. Fixed value for all chirps: To generate sequence of chirps which never changes, then only
 *      one value can be programmed in LUT (LUT Dither), i.e NUM_OF_PATTERNS (P) = 1 and
 *      LUT_PARAM_UPDATE_PERIOD (K) = 0 \n
 *   2. Unique chirps: Index every LUT_PARAM_UPDATE_PERIOD (K) chirps in LUT to generate unique
 *      sequence of chirps. \n
 *   3. Delta increment every DELTA_PARAM_UPDATE_PERIOD (N) chirps: On top of sequence of unique
 *      chirps from LUT, the fixed delta increment (Delta dither) can be done every N chirps. \n
 *   4. The set of chirp parameters across bursts and sub-frames can be different by setting offset
 *      to LUT in BURST_LUT_INDEX_OFFSET and SF_LUT_INDEX_OFFSET. \n
 *
 *   @note 1 : The Legacy \ref rlSetChirpConfig, \ref rlSetDynChirpCfg, \ref rlRfSetPhaseShiftConfig
 *             , \ref rlSetDynPerChirpPhShifterCfg and \ref rlSetBpmChirpConfig APIs are not
 *             supported if device is configured with Advanced Chirp Config API enabled in
 *             AWR_RF_RADAR_MISC_CTL_SB or vice versa. \n
 *   @note 2 : The per chirp phase shifter and BPM configurations are part of this API. \n
 *   @note 3 : The parameters in this API are not applicable to loop-back sub-frames
 *             \ref rlSetLoopBckBurstCfg. If loop-back sub-frames are needed, it is recommended to
 *             be configured in the last sub-frame (SF) of \ref rlSetAdvFrameConfig API. \n
 *   @note 4 : The dynamic update of this API is allowed at frame boundary along with the Generic
 *             SW Chirp Parameters, as long as the LUT addresses modified differ from the addresses
 *             used in the current ongoing frame. The dynamic chirp enable
 *             API AWR_DYN_CHIRP_ENABLE_SB shall be issued at least 500us before end of current
 *             active window of frame (500us before start of idle time of the frame) to apply the
 *             dynamic configurations in immediate next frame. \n
 *   @note 5 : If fixed delta dither is used to generate the pattern then it is recommended to
 *             program same start frequency in profile config API for each chirps in a frame. Each
 *             chirp can have different profiles associated with it except start frequency. \n
 *   @note 6 : The number of chirps programmed in a burst/frame shall be multiple of 4. \n
 *             Exception: a single chirp can be programmed in a burst. \n
 *   @note 7 : The RF frequency used for measurement in monitors are derived only from profile
 *             settings (start frequency and slope) and not from the advance chirp
 *             configuration API, if fixed delta increment is used to change the start frequency
 *             every chirp, it is recommended to have a separate profile for monitors which covers
 *             full RF bandwidth of interest. \n
 *   @note 8 : All the fields in this API are specific to selected CHIRP_PARAM_INDEX in this API,
 *             this API needs to be programmed ten times for each of the chirp parameters defined
 *             in CHIRP_PARAM_INDEX field in below API. \n
 *   @note 9 : The Delta Dither is optional and can be disabled by setting
 *             DELTA_PARAM_UPDATE_PERIOD (N) = 0 and SFn_CHIRP_PARAM_DELTA = 0. \n
 *   @note 10: The LUT Dither is mandatory and at least one dither parameter value (it can be value
 *             zero) shall be programmed for all chirp parameters in generic LUT, same dither value
 *             can be programmed to all chirps in a burst/frame by setting
 *             LUT_PARAM_UPDATE_PERIOD (K) = 0. \n
 */
/* DesignId : MMWL_DesignId_129 */
/* Requirements : AUTORADAR_REQ-1050, AUTORADAR_REQ-1043 */
rlReturnVal_t rlSetAdvChirpConfig(rlUInt8_t deviceMap, rlAdvChirpCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetAdvChirpConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetAdvChirpConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_ADV_CHIRP_CFG_SET_SB,
                                       (rlUInt8_t *)(data),
                                       (rlUInt16_t)sizeof(rlAdvChirpCfg_t));
    }

    RL_LOGV_ARG0("rlSetAdvChirpConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlMonTypeTrigConfig(rlUInt8_t deviceMap, rlMonTypeTrigCfg_t *data)
 *
 *   @brief Maintains monitoring timing synchronization in cascaded devices to avoid mutual
 *          interference of monitors running in different devices in the cascade sensor.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - structure contains monitor trigger type configs
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API helps to maintain monitoring timing synchronization in cascaded devices to avoid
 *   mutual interference of monitors running in different devices in the cascade sensor. \n
 *   The host must trigger the monitor of types below to avoid interference if MONITORING MODE
 *   is set to ’1’ in AWR_CALIB_MON_TIME_UNIT_CONF_SB. \n
 *   The AWR_AE_RF_MONITOR_TYPE_TRIGGER_DONE_SB_AE will be sent once monitor type is executed \n
 *   The monitors can be categorized into 3 types : \n
 *
 *   Monitor Types  Description
 *      Type 0      Non-transmitting monitor, The execution of non-transmitting monitors does not
 *                  cause RF interference to monitors executing on other devices. Therefore, they
 *                  can be executed in parallel across all devices in the cascade. These include
 *                  monitors which receive a test signal through RX LNA and digital monitors. \n
 *      Type 1      Transmitting but not receiving (test signal), The monitors that transmit but
 *                  don’t receive any test signal through RX LNA are not susceptible to
 *                  interference. Therefore, they can be executed in parallel across all devices
 *                  in the cascade, but not when monitors that receive test signals through RX
 *                  LNA are executing. \n
 *      Type 2      Transmitting and receiving (test signal), The monitors that transmit and also
 *                  receive test signal through RX LNA are susceptible to interference. They can
 *                  be executed sequentially so as to create time separation between monitoring
 *                  chirps of different devices \n
 *
 *   @note 1: This is a new feature addition in AWR2243. \n
 *   @note 2: The Host can trigger all 3 types of monitor at same time or can trigger each type
 *   one after other based on system requirement, in case host is triggering monitor types one
 *   after other, then it is recommended to follow order type 0, type 1 and type 2 respectively. \n
 *   @note 3: The Host must wait for AWR_AE_RF_MONITOR_TYPE_TRIGGER_DONE_SB_AE before issuing
 *   trigger for next monitor type. \n
 *   @note 4: The Host must ensure all types of monitors are executed within defined device
 *   FTTI interval, otherwise device can not finish all the monitors within FTTI and will report
 *   failure AE_AWR_CAL_MON_TIMING_FAIL_REPORT_AE_SB \n
 */
/* DesignId : MMWL_DesignId_125 */
/* Requirements : AUTORADAR_REQ-1055 */
rlReturnVal_t rlMonTypeTrigConfig(rlUInt8_t deviceMap, rlMonTypeTrigCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlMonTypeTrigConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlMonTypeTrigConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_MONITOR_TYPE_TRIG_CONF_SB,
                                       (rlUInt8_t *)(data),
                                       (rlUInt16_t)sizeof(rlMonTypeTrigCfg_t));
    }

    RL_LOGV_ARG0("rlMonTypeTrigConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlRfApllSynthBwCtlConfig(rlUInt8_t deviceMap,
                                             rlRfApllSynthBwControl_t* data)
*
*   @brief Control bandwidth of the APLL and Synthesizer
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - APLL and Synthesizer B/W control data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API is used to control bandwidth of the APLL and Synthesizer
*
*   @note 1 : This API is supported only in AWR2243. \n
*   @note 2 : Recommended to issue this API before \ref rlRfInit API. The RF_INIT synthesizer
*             boot calibration shall run after changing the APLL BW. \n
*/
/* DesignId : MMWL_DesignId_126 */
/* Requirements : AUTORADAR_REQ-1053 */
rlReturnVal_t rlRfApllSynthBwCtlConfig(rlUInt8_t deviceMap, rlRfApllSynthBwControl_t *data)
{
    rlReturnVal_t retVal;

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_STATIC_CONF_SET_MSG,
                                       RL_RF_APLL_SYNTH_BW_CTL_SB,
                                       (rlUInt8_t *)data,
                                       (rlUInt16_t)sizeof(rlRfApllSynthBwControl_t));
    }
    return retVal;
}

/** @fn rlReturnVal_t rlSetAdvChirpLUTConfig(rlUInt8_t deviceMap, rlAdvChirpLUTCfg_t *data)
 *
 *   @brief Set the Advanced chirp LUT configuration to the device.
 *   @param[in] deviceMap - Bitmap of devices to send the message
 *   @param[in] data - structure contains adv-chirp configs
 *
 *   @return rlReturnVal_t Success - 0, Failure - Error Code
 *
 *   This API sub-block loads the unique dither values for each chirp on Generic SW Chirp Parameter
 *   LUT at offset address defined in AWR_ADVANCE_CHIRP_CONF_SB API. This LUT can be used to
 *   pre-load dither patterns for each chirp parameters and provides the flexibility to program any
 *   number of unique dithers for each chirp parameters. \n
 *
 *   @note 1 : The Generic SW Chirp Parameter LUT can be modified by the host dynamically, as long
 *             as the LUT addresses modified differ from the addresses used in the current frame. \n
 *   @note 2 : The dynamic update of this API is effective immediately and does not depend on
 *             AWR_DYN_CHIRP_ENABLE_SB API. This might impact the ongoing chirps if timing of the
 *             update is not handled properly as if ongoing chirps use same fields/addresses in LUT.
 *             It is recommended to perform proper timing analysis before updating the LUT
 *             dynamically considering SPI communication delays. \n
 *   @note 3 : The total size of Generic SW Chirp Parameter LUT is 12kB. \n
 *   @note 4 : The start address offset of all chirp parameter in LUT shall be multiple of 4 bytes
 *             (word boundary), that means minimum 4 bytes in LUT shall be allocated to each chirp
 *             parameter. \n
 *   @note 5 : At least one dither parameter value shall be programmed for each chirp parameter type
 *             (10 types) in generic LUT, same value can be programmed to all chirps in a
 *             burst/frame using Advance chirp config API, LUT_PARAM_UPDATE_PERIOD (K) = 0
 *             configuration\n
 */
/* DesignId : MMWL_DesignId_130 */
/* Requirements : AUTORADAR_REQ-1050 */
rlReturnVal_t rlSetAdvChirpLUTConfig(rlUInt8_t deviceMap, rlAdvChirpLUTCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetAdvChirpLUTConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetAdvChirpLUTConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_ADV_CHIRP_CFG_LUT_SET_SB,
                                       (rlUInt8_t *)(data),
                                       (rlUInt16_t)sizeof(rlAdvChirpLUTCfg_t));
    }

    RL_LOGV_ARG0("rlSetAdvChirpLUTConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetMultiAdvChirpLUTConfig(rlUInt8_t deviceMap,
                                                  rlAdvChirpLUTCfg_t* AdvChirpLUTCfgArgs,
                                                  rlInt8_t* AdvChirpLUTData)
*
*   @brief Multi Advanced chirp LUT configuration API
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] AdvChirpLUTCfgArgs - Parameters for Advanced chirp LUT configuaration
*   @param[in] AdvChirpLUTData - LUT buffer filled with the chirp parameter data
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   Multi Advanced chirp LUT configuration API.
*   This API is used to send the programmed LUT RAM data to the device populating the RadarSS LUT.
*   This API takes Chirp LUT buffer from the application and internally chunks to multiple SPI
*   packets (max 256 Bytes each).
*
*   @note : This API is supported only in AWR2243. \n
*/
/* DesignId : MMWL_DesignId_131 */
/* Requirements : AUTORADAR_REQ-1050 */
/*AR_CODE_REVIEW MR:R.2.2 <APPROVED> "numBytes is re initialized under different conditions" */
/*LDRA_INSPECTED 8 D */
rlReturnVal_t rlSetMultiAdvChirpLUTConfig(rlUInt8_t deviceMap,
                                          rlAdvChirpLUTCfg_t *AdvChirpLUTCfgArgs,
                                          rlInt8_t *AdvChirpLUTData)
{
    rlReturnVal_t retVal = RL_RET_CODE_OK;
    /* Structure used to send the LUT RAM data to populate the LUT at the device end */
    rlAdvChirpLUTCfg_t AdvChirpLUTCfgChunk = {0};
    /* Variables to keep track of the number of chunks and the chunk size */
    rlUInt16_t chunkCount = 0U, totalChunkCount, lastChunkSize;

    RL_LOGV_ARG0("rlSetMultiAdvChirpLUTConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) ||
        (RL_NULL_PTR == AdvChirpLUTCfgArgs) || (RL_NULL_PTR == AdvChirpLUTData))
    {
        /* set return error code */
        retVal += RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetMultiAdvChirpLUTConfig, Invalid input params\n");
    }
    else
    {
        RL_LOGV_ARG2("rlSetMultiAdvChirpLUTConfig with \nlutAddOffset[%d]\nnumBytes[%d] \n\n",
                     AdvChirpLUTCfgArgs->lutAddressOffset, AdvChirpLUTCfgArgs->numBytes);

        /* If the LUT size to be programmed is more than 212 bytes, then break them into chunks
           of 212 bytes and send them one after the other over SPI */
        if (AdvChirpLUTCfgArgs->numBytes > RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE)
        {
            /* If the overall LUT size is a multiple of 212 bytes */
            if ((AdvChirpLUTCfgArgs->numBytes % RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE) == 0U)
            {
                lastChunkSize = RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE;
                totalChunkCount =
                    (rlUInt16_t)(AdvChirpLUTCfgArgs->numBytes / RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE);
            }
            /* Else, keep track of the remaining bytes in the last chunk */
            else
            {
                totalChunkCount =
                    (rlUInt16_t)((AdvChirpLUTCfgArgs->numBytes / RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE) + 1U);
                lastChunkSize =
                    (rlUInt16_t)(AdvChirpLUTCfgArgs->numBytes % RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE);
            }
        }
        /* If the total size of the LUT to be programmed is less than 212 bytes, it can be
           sent in a single chunk */
        else
        {
            totalChunkCount = 1U;
            /* last chunk size is same as total number of bytes in this case */
            lastChunkSize = AdvChirpLUTCfgArgs->numBytes;
        }

        while (chunkCount < totalChunkCount)
        {
            RL_LOGV_ARG1("Sending Advanced Chirp LUT data Chunk [%d]... \n\n", chunkCount);
            /* Update the LUT Address offset for every chunk */
            AdvChirpLUTCfgChunk.lutAddressOffset =
                (rlUInt16_t)(AdvChirpLUTCfgArgs->lutAddressOffset +
                             (chunkCount * RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE));

            /* If the data belongs to last chunk or if it is a single chunk */
            if (chunkCount == (totalChunkCount - 1U))
            {
                AdvChirpLUTCfgChunk.numBytes = lastChunkSize;
            }
            /* For other intermediate chunks, send the entire length of 212 bytes */
            else
            {
                AdvChirpLUTCfgChunk.numBytes = RL_MAX_ADV_CHIRP_LUT_CHUNK_SIZE;
            }

            if (AdvChirpLUTCfgChunk.numBytes > 0U)
            {
                /* Copy the input AdvChirpLUTData to LUT structure */
                /*AR_CODE_REVIEW MR:R.18.1 <INSPECTED> "Accessing pointer as array"*/
                /*LDRA_INSPECTED 436 S */
                (void)memcpy(&AdvChirpLUTCfgChunk.calData[0U],
                             &AdvChirpLUTData[AdvChirpLUTCfgChunk.lutAddressOffset],
                             AdvChirpLUTCfgChunk.numBytes);

                RL_LOGV_ARG2("rlSetAdvChirpLUTConfig with \nlutAddOffset[%d]\nnumBytes[%d] \n\n",
                             AdvChirpLUTCfgChunk.lutAddressOffset, AdvChirpLUTCfgChunk.numBytes);
                /* Send the LUT RAM structure (chunk) to the device */
                retVal += rlSetAdvChirpLUTConfig(deviceMap, &AdvChirpLUTCfgChunk);

                if (retVal != RL_RET_CODE_OK)
                {
                    RL_LOGV_ARG2("Advanced Chirp LUT data Chunk [%d] failed with error code %d",
                                 chunkCount, retVal);
                    break;
                }
            }
            else
            {
                /* set return error code */
                retVal += RL_RET_CODE_INVALID_INPUT;
                RL_LOGV_ARG2("Invalid input, Chirp LUT Chunk [%d] is programmed with %d bytes",
                             chunkCount, AdvChirpLUTCfgChunk.numBytes);
            }

            chunkCount++;
        }
    }

    RL_LOGV_ARG0("rlSetMultiAdvChirpLUTConfig ends...\n");

    return retVal;
}

/** @fn rlReturnVal_t rlSetAdvChirpDynLUTAddrOffConfig(rlUInt8_t deviceMap, \
                                                       rlAdvChirpDynLUTAddrOffCfg_t *data)
*
*   @brief Configure LUT address offset dynamically for each chirp parameters defined in
*          \ref rlSetAdvChirpConfig API.
*   @param[in] deviceMap - Bitmap of devices to send the message
*   @param[in] data - structure contains adv-chirp dynamic LUT address offset configs
*
*   @return rlReturnVal_t Success - 0, Failure - Error Code
*
*   This API helps to update only the LUT offset address for chirp parameters when LUT data is
*   modified at frame boundary dynamically while frames are running. \n
*   The dynamic chirp enable API AWR_DYN_CHIRP_ENABLE_SB shall be issued after issuing this API
*   at least 500us before end of current active window of frame (500us before start of idle time
*   of the frame) to apply the dynamic configurations in immediate next frame. \n
*/
/* DesignId : MMWL_DesignId_132 */
/* Requirements : AUTORADAR_REQ-1050 */
rlReturnVal_t rlSetAdvChirpDynLUTAddrOffConfig(rlUInt8_t deviceMap,
                                               rlAdvChirpDynLUTAddrOffCfg_t *data)
{
    rlReturnVal_t retVal;

    RL_LOGV_ARG0("rlSetAdvChirpDynLUTAddrOffConfig starts...\n");

    /* check if deviceIndex is out of defined value */
    if ((rlDriverIsDeviceMapValid(deviceMap) != RL_RET_CODE_OK) || (RL_NULL_PTR == data))
    {
        /* set return error code */
        retVal = RL_RET_CODE_INVALID_INPUT;
        RL_LOGE_ARG0("rlSetAdvChirpDynLUTAddrOffConfig, Invalid device map\n");
    }
    else
    {
        /* Package the command with given data and send it to device */
        retVal = rlDriverExecuteSetApi(deviceMap, RL_RF_DYNAMIC_CONF_SET_MSG,
                                       RL_RF_ADV_CHIRP_DYN_LUT_ADD_OFF_CONF_SB,
                                       (rlUInt8_t *)(data),
                                       (rlUInt16_t)sizeof(rlAdvChirpDynLUTAddrOffCfg_t));
    }

    RL_LOGV_ARG0("rlSetAdvChirpDynLUTAddrOffConfig ends...\n");

    return retVal;
}

/*
 * END OF rl_sensor.c FILE
 */
