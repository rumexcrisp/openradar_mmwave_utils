# Install instructions

There are a few dependencies to using this library. 

## Dependencies

You'll need FD2XX. Download it from the ftdi website:

Linux desktop: [libftd2xx-x86_64-1.4.27.tgz](https://ftdichip.com/wp-content/uploads/2022/07/libftd2xx-x86_64-1.4.27.tgz)

If you are uncertain, then just go to: [https://www.ftdichip.com/Drivers/D2XX.htm](https://ftdichip.com/drivers/d2xx-drivers/)

Then you'll need to install the precompiled library.

```bash
cd ~/Downloads
mkdir libftdi
tar -zxvf libftd2xx-x86_64-1.4.27.tg -C ./libftdi
cd ./libftdi
sudo cp ftd2xx.h /usr/include  
sudo cp WinTypes.h /usr/include  
cd /usr/local/include  
sudo ln -s /usr/include/ftd2xx.h ftd2xx.h
sudo ln -s /usr/include/WinTypes.h WinTypes.h
cd ~/Downloads/libftdi/release/build
sudo cp libftd2xx-x86_64-1.4.27 /usr/local/lib
cd /usr/local/lib
sudo ln -s libftd2xx-x86_64-1.4.27 libftd2xx.so
cd /usr/lib
sudo ln -s /usr/local/lib/libftd2xx-x86_64-1.4.27 libftd2xx.so
```

## Possible dependencies

```bash
sudo apt-get install libtool
sudo apt-get install autoconf
sudo apt-get install texinfo
sudo apt-get install libusb-dev
```
