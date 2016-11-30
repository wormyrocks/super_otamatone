# super_otamatone

Because the world needs a better Otamatone.

In order to set up build environment, you must have three .mk files from the Arduino Makefile repository in arduino/build_scripts

Teensy.mk
Common.mk
Arduino.mk

Find them here.
https://github.com/sudar/Arduino-Makefile

You also need the Arduino application and Teensyduino, as well as the libraries specified in the Makefile.

This library uses my fork of the arduino-menusystem library, which can be found here.
https://github.com/wormyrocks/arduino-menusystem
It is included as a submodule.

'build' script automates concatenating the .ino files in /src, as well as uploading to Teensy.
I use tyc to upload to Teensy. Find binaries here. https://github.com/Koromix/ty/releases
tyc should also be in this directory.
