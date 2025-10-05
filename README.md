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
