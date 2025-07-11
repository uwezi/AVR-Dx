/// \file i2c_lcd.h
/// \brief LiquidCrystal library with PCF8574 I2C adapter.
///
/// \author Uwe Zimmermann
/// \remark based on C++ library by Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2019 by Matthias Hertel.
/// \copyright Copyright (c) 2025 by Uwe Zimmermann

/// The original library work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
/// This library can drive a Liquid Crystal Display (LCD) based on the Hitachi HD44780 chip that is connected
/// through a PCF8574 I2C adapter.
/// The API is common to many LCD libraries and documented in https://www.arduino.cc/en/Reference/LiquidCrystal.
/// and partially functions from https://playground.arduino.cc/Code/LCDAPI/.

///
/// ChangeLog:
/// --------
/// * 19.10.2013 created.
/// * 05.06.2019 rewrite from scratch.
/// * 26.05.2022 8-bit datatypes in interfaces and compatibility topics.
/// * 26.05.2022 createChar with PROGMEM character data for AVR processors.
/// * 26.05.2022 constructor with pin assignments. Thanks to @markisch.
/// * 09.07.2025 ported to plain C and AVR-Dx series, Uwe Zimmermann

#ifndef i2c_lcd_h
#define i2c_lcd_h

#include <avr/io.h>
#include <stddef.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

void LCD_init(uint8_t cols, uint8_t lines, TWI_t *twi,
              uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t e,
              uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight);

void LCD_clear();
void LCD_home();
void LCD_setCursor(uint8_t col, uint8_t row);
void LCD_cursor();
void LCD_noCursor();
void LCD_blink();
void LCD_noBlink();
void LCD_display();
void LCD_noDisplay();
void LCD_scrollDisplayLeft();
void LCD_scrollDisplayRight();
void LCD_autoscroll();
void LCD_noAutoscroll();
void LCD_leftToRight();
void LCD_rightToLeft();
void LCD_createChar(uint8_t location, uint8_t charmap[]);
void LCD_createChar_P(uint8_t, const uint8_t *data);
void LCD_print(char *ch);

// plus functions from LCDAPI:
void LCD_setBacklight(uint8_t brightness);
inline void LCD_command(uint8_t value) { _send(value); }

// support of Print class
size_t LCD_write(uint8_t ch);

// instance variables
uint8_t LCD_i2cAddr;        ///< Wire Address of the LCD
uint8_t LCD_backlight;      ///< the backlight intensity
uint8_t LCD_cols;           ///< number of cols of the display
uint8_t LCD_lines;          ///< number of lines of the display
uint8_t LCD_entrymode;      ///< flags from entrymode
uint8_t LCD_displaycontrol; ///< flags from displaycontrol
uint8_t LCD_row_offsets[4];

// variables describing how the PCF8574 is connected to the LCD
uint8_t LCD_rs_mask;
uint8_t LCD_rw_mask;
uint8_t LCD_enable_mask;
uint8_t LCD_backlight_mask;
// these are used for 4-bit data to the display.
uint8_t LCD_data_mask[4];

// low level functions
uint8_t LCD_send(uint8_t value, uint8_t isData);

void LCD_sendNibble(uint8_t halfByte, uint8_t isData);

uint8_t LCD_writeNibble(uint8_t halfByte, uint8_t isData);

void LCD_write2Wire(uint8_t data, uint8_t isData, uint8_t enable);
static uint8_t i2c_0_WaitR(void);
#endif