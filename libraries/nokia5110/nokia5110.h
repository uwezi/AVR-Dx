/**
 * @file nokia5110.h
 * @brief simple library for the Nokia 5110 transflexive LCD, 84x48 pixels
 *
 * @author Uwe Zimmermann
 *
 * The library work is licensed under a MIT license.\n
 * See https://github.com/uwezi/AVR-Dx
 *
 * ChangeLog:
 * --------
 * * 2015-06-09 originally created
 * * 2025-09-05 ported to the AVR-Dx family
 */
#ifndef NOKIA5110_H_
#define NOKIA5110_H_

#include <avr/io.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

#define NOKIA_SIZEX 84
#define NOKIA_SIZEY 48
#define NOKIA_ORIENTATION_0   0
#define NOKIA_ORIENTATION_180 1
#define NOKIA_NORMAL    0
#define NOKIA_INVERSE   1
#define NOKIA_UNDERLINE 2

/**
 * @name smallFont
 * @brief a 6x8 pixel font
 */
static const uint8_t smallFont[][6] PROGMEM =
#include "font_6x8_iso8859_1.h"

extern uint8_t NOKIA_FRAMEBUFFER[NOKIA_SIZEX*NOKIA_SIZEY/8];
extern uint8_t NOKIA_ORIENTATION;

/**
 * @name NOKIA_writeCommand
 * @param data uint8_t of data
 * @return none
 * @brief writes one byte in command-mode to the display
 */
void NOKIA_writeCommand (uint8_t command );

/**
 * @name NOKIA_writeData
 * @param data uint8_t of data
 * @return none
 * @brief writes one byte in data-mode to the display
 */
void NOKIA_writeData (uint8_t data );

/**
 * @name NOKIA_gotoXY
 * @param x x-coordinate 0..83
 * @param y y-coordinate 0..47
 * @return none
 * @brief sets the displays write pointer - for partial writes (not used)
 */
void NOKIA_gotoXY ( uint8_t x, uint8_t y );

/**
 * @name NOKIA_fillbuffer
 * @param value byte value to fill the display with
 * @return none
 * @brief fills the display buffer with a given bit pattern
 */
void NOKIA_fillbuffer(uint8_t value);

/**
 * @name NOKIA_clearbuffer
 * @param none
 * @return none
 * @brief clears the display buffer (no update to the display)
 */
void NOKIA_clearbuffer(void);

/**
 * @name NOKIA_update
 * @param none
 * @return none
 * @brief updates the display with the current framebuffer
 */
void NOKIA_update (void);

/**
 * @name NOKIA_clear
 * @param none
 * @return none
 * @brief clears the display buffer and updates the display
 */
void NOKIA_clear ( void );

/**
 * @name NOKIA_init
 * @param sce_port PORT-struct of the SCE pin
 * @param sce_pin pin number of the SCE pin
 * @param rst_port PORT-struct of the RST pin
 * @param rst_pin pin number of the RST pin
 * @param dc_port PORT-struct of the DC pin
 * @param dc_pin pin number of the DC pin
 * @param sd_port PORT-struct of the SD/DIN pin
 * @param sd_pin pin number of the SD/DIN pin
 * @param scl_port PORT-struct of the SCL/CLK pin
 * @param scl_pin pin number of the SCL/CLK pin
 * @param vop  the contrast voltage parameter 0..127
 * @param orientation  orientation of the display (0: 0°, 1: 180°)
 * @return none
 * @brief initialize the NOKIA display
 */
void NOKIA_init (
  volatile PORT_t *sce_port, uint8_t sce_pin,
  volatile PORT_t *rst_port, uint8_t rst_pin,
  volatile PORT_t *dc_port, uint8_t dc_pin,
  volatile PORT_t *sd_port, uint8_t sd_pin,
  volatile PORT_t *scl_port, uint8_t scl_pin,
  uint8_t vop,
  uint8_t orientation
);

/**
 * @name NOKIA_setVop
 * @param vop  the contrast voltage parameter 0..127
 * @return none
 * @brief sets the display's contrast voltage
 */
void NOKIA_setVop(uint8_t vop);

/**
 * @name NOKIA_setpixel
 * @param x  x-coordinate 0..83
 * @param y  y-coordinate 0..47
 * @return none
 * @brief sets a single pixel at (x,y)
 */
void NOKIA_setpixel(uint8_t x, uint8_t y);

/**
 * @name NOKIA_clearpixel
 * @param x  x-coordinate 0..83
 * @param y  y-coordinate 0..47
 * @return none
 * @brief clears a single pixel at (x,y)
 */
void NOKIA_clearpixel(uint8_t x, uint8_t y);

/**
 * @name NOKIA_putchar
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param ch  character 0..255
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief puts a single character onto LCD
 */
void NOKIA_putchar(uint8_t x0, uint8_t y0, char ch, uint8_t attr);

/**
 * @name NOKIA_print
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string into the framebuffer
 */
void NOKIA_print(uint8_t x, uint8_t y, char *ch, uint8_t attr);

/**
 * @name NOKIA_print_p
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string in PROGMEM
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string from PROGMEM into the framebuffer
 */
void NOKIA_print_p(uint8_t x, uint8_t y, const char *ch,uint8_t attr);

/**
 * @name NOKIA_scroll
 * @param dy  y-shift -47..47
 * @return none
 * @brief shifts the contents of the framebuffer vertically
 */
void NOKIA_scroll(int8_t dy);

#endif /* NOKIA5110_H_ */
