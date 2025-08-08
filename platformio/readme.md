# Settings for the platformio extension in VSCode

When removing the link to the Arduino framework from the platformio.ini, PlatformIO reverts back to the older 1.7 toolchain, which still lacks support for the AVR-Dx microcontrollers. You can override this in the platfromio.ini:

```
[env:AVR64DA28]
platform_packages = platformio/toolchain-atmelavr@^3.70300.220127
platform = atmelmegaavr
board = AVR64DA28
board_build.f_cpu = 4000000UL
upload_protocol = custom
upload_flags = -cserialupdi
  -p64da28
  -PCOM4
upload_command = "C:\Program Files (x86)\AVRDUDESS\avrdude.exe" $UPLOAD_FLAGS -U flash:w:$SOURCE:i
```

But there is another uption as well, you can change the file `.platformio/platforms/atmelmegaavr/platform.json`

from
```
  "packages": {
    "toolchain-atmelavr": {
      "type": "toolchain",
      "owner": "platformio",
      "version": "~1.70300.0",
      "optionalVersions": ["~3.70300.0"]
    },
```
to
```
  "packages": {
    "toolchain-atmelavr": {
      "type": "toolchain",
      "owner": "platformio",
      "version": "~3.70300.0"
    },
```
and then have your platformio.ini simply as
```
[env:AVR64DA28]
platform = atmelmegaavr
board = AVR64DA28
board_build.f_cpu = 4000000UL
upload_protocol = custom
upload_flags = -cserialupdi
  -p64da28
  -PCOM4
upload_command = "C:\Program Files (x86)\AVRDUDESS\avrdude.exe" $UPLOAD_FLAGS -U flash:w:$SOURCE:i
```

Finally you can create a new board manifest in order to default to the `serialupdi` upload protocol and the 4 MHz default clock frequency:

file `.platformio/platforms/atmelmegaavr/boards/AVR64DA28_serialupdi.json`
```
{
  "build": {
    "f_cpu": "4000000L",
    "mcu": "avr64da28",
    "variant": "28pin-standard"
  },
  "hardware": {
    "oscillator": "internal"
  },
  "name": "AVR64DA28 serial UPDI",
  "upload": {
    "maximum_ram_size": 8192,
    "maximum_size": 65536,
    "protocol": "serialupdi"
  },
  "url": "https://www.microchip.com/wwwproducts/en/AVR64DA28",
  "vendor": "Microchip"
}
```

now you can shrink your `platformio.ini` to
```
[env:AVR64DA28_serialupdi]
platform = atmelmegaavr
board = AVR64DA28_serialupdi
```
