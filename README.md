# Open Radar mmwave utils

Repository for TI MMWave utils and simple processing.  
This repository contains:

* A dirty linux port of TI's MMWave example
* A dirty linux port of TI's DCA1000 example
* Example python sofware for radar processing using CUPY

## Setup and get started

Perform the following steps to get started:

### Install dependencies

Install the dependencies as described in [installation instructions](./docs/install.md).

### Connect the radar

* Connect the micro-USB port on the DCA1000 to your system
* Connect the AWR2243 to a 5V barrel jack
* Set power connector on the DCA1000 to **RADAR_5V_IN**
* Put the device in SOP0
  * Jumper on SOP0, all others disconnected
* Connect the RJ45 to your system
* **Set a fixed IP to the local interface: 192.168.33.30**

### Fix the user privileges

```bash
sudo touch /etc/udev/rules.d/99-libftdi.rules 
sudo vim /etc/udev/rules.d/99-libftdi.rules 
```

Add the following line:

```sh
SUBSYSTEM=="usb", ATTR{idVendor}=="0451", ATTR{idProduct}=="fd03", GROUP="usb", MODE="0664"
```

Then create the group and add your user:

```bash
sudo useradd -G usb $USER$
sudo usermod -a -G usb $USER$
```

### Build the code

1. Create and enter build folder: `mkdir build && cd build`
2. Setup cmake, use Ninja for generator: `cmake -G Ninja ..`
3. Build source: `ninja`

You will find two binaries `setup_dca_1000` and `setup_radar` inside a newly created bin folder.