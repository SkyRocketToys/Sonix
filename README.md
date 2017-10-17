Ftirmware for video board on skyviper
====================================

This is the firmware source for the ArduPilot based SkyViper GPS copter. It is based on FreeRTOS

Building
--------

To build the firmware follow these steps
- cd buildscript
- git submodule init
- git submodule update
- make oldconfig
- make clean
- make mavlink
- make
- make install

That will create a file image/dashcam/FIRMWARE_660R.bin which you can
upload to the SkyViper using the web interface.

Note that you will need the i386 lib libraries installed if you are on a
64 bit Linux varient. That usually means you need the libc6:i386 and zliblg:i386
package installed. On Debian/Ubuntu:

- sudo apt-get install libc6:i386
- sudo apt-get install zlib1g:i386

You may also need the following Perl libraries:

- sudo apt-get install libencode-detect-perl
- sudo apt-get install libdigest-crc-perl
