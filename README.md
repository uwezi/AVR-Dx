# AVR-Dx / AVR-EA
some general stuff about the AVR-DA, AVR-DB, AVR-DD and AVR-EA series

## What and Why
As I am just getting started with the "new" Microchip AVR-Dx series of microcontrollers which seem to be replacing the good old ATmega328 workhorse and its tiny and mega friends for me, I plan to collect some usefull notes here.

## Libraries:
- [DS18B20](libraries/DS18B20) - a library to use the 1-wire digital thermometer DS18B20
- [lcd](libraries/lcd) - a library for a HD44780-based alphanumeric display, based on Peter Fleury's LCD library
- [lcd_i2c](libraries/lcd_i2c) - a library to use an alphanumeric display with a serial I2C port extender board
- [modbus_rtu](libraries/modbus_rtu) - basic functionality for a RS485 Modbus/RTU slave
- [nokia5110](libraries/nokia5110) - simple library for text and graphics on a Nokia 5110 LCD with 84x48 pixels

## Hardware
- [pinouts](pinouts) - an overview over the pinouts of selected AVR-Dx chips
- [UPDI + UART](UPDI_plus_UART) - a breadbaord UPDI programmer and UART interface

<img width="672" height="568" alt="image" src="https://github.com/user-attachments/assets/d81acdd6-1bb7-4d86-88f5-67d012e74c1d" />

- [UPDI Programmer](UPDI_programmer) - how to convert a USB-C UART module into an UPDI programmer and the attached pitfalls

## Observations and errata
- On both the AVR32DA28 and AVR128DA64, purchased in summer 2025, the RESET pin was disabled in the `syscfg0` fuse register by default. This is in contrast to the corresponding section from both devices' datasheets. The value in the fuse register on both factory-new devices was `0xC0` and not `0xC8`. 
> The default value given in this fuse description is the factory-programmed value, and should not be mistaken for the
Reset value.

<img width="1018" height="512" alt="bild" src="https://github.com/user-attachments/assets/5812fdbc-f252-4d19-bbbb-dd6488b43fd3" />

Setting the fuse to the correct value for enabling the reset pin can be done by
```avrdude -p32da28 -PCOM3 -cserialupdi -U syscfg0:w:0xc0:m```


