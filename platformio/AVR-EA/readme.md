The AVR-EA series is currently not yet supported by the latest release of the avr-gcc toolchain by Microchip. 
I did not get the provided device packs by Microchip to work in the PlatformIO environment, but here is a working solution for now:

- get the modified avr-gcc toolchain from the maker of the DXCore (https://github.com/SpenceKonde/DxCore), for Windows this is
https://spencekondetoolchains.s3.amazonaws.com/avr-gcc-7.3.0-atmel3.6.1-azduino7b-i686-w64-mingw32.tar.bz2
- replace the currently installed megaavr toolchain in .platformio/packages/toolchain-atmelavr
- replace the provided avrdude v6.3 with a more recent avrdude in .platformio/packages/tool-avrdude-megaavr
- store the board description file AVR64EA28.json in .platformio/platforms/atmelmegaavr/boards
- add the line `platform_packages = platformio/toolchain-atmelavr` to your project's platformio.ini
