/**
 * @file nokia5110_hspi.c
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
 * * 2025-10-20 added tiny font
 */

#include "nokia5110_hspi.h"

/**
 * @brief globals
 */
typedef struct
{
  volatile PORT_t *Port;
  uint8_t Pin_bm;
} PIN_t;

PIN_t NOKIA_RST, NOKIA_DC;
SPI_t *NOKIA_SPI;

/**
 * @name NOKIA_FRAMEBUFFER
 * @brief a software framebuffer
 */
uint8_t NOKIA_FRAMEBUFFER[NOKIA_SIZEX*NOKIA_SIZEY/8]= {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF,
0xFF, 0xC0, 0x80, 0x80, 0xC0, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFC, 0xFE, 0xFE, 0x86, 0x86, 0xC6,
0xFE, 0xFC, 0x78, 0x00, 0x00, 0xFC, 0xFE, 0xFE, 0x86, 0x86, 0xC6, 0xFE, 0xFC, 0x78, 0x00, 0x80,
0x8C, 0x9E, 0xBE, 0xF6, 0xE6, 0xE6, 0x00, 0xC0, 0xE6, 0xE6, 0xB6, 0xB6, 0xB6, 0xFE, 0xFE, 0xFC,
0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xC0, 0xE6, 0xE6, 0xB6, 0xB6, 0xB6, 0xFE, 0xFE, 0xFC, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F,
0x1F, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x00, 0x01, 0x01, 0x01,
0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00,
0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE,
0xFE, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0x60, 0x30, 0x30,
0xF0, 0xF0, 0xE0, 0x00, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0x10, 0xF0, 0xF0, 0xC0, 0x00, 0xC0, 0xF0,
0xF0, 0x10, 0xC0, 0xE0, 0xF0, 0x30, 0x30, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0x60,
0x30, 0x30, 0x00, 0x60, 0xF0, 0xF0, 0xB0, 0x30, 0x30, 0x00, 0x00, 0xF6, 0xF6, 0xF6, 0x00, 0x30,
0xFC, 0xFC, 0xFE, 0x30, 0x30, 0x00, 0x10, 0xF0, 0xF0, 0xC0, 0x00, 0xC0, 0xF0, 0xF0, 0x10, 0x00,
0x00, 0x00, 0x03, 0x07, 0x0F, 0x0E, 0x0C, 0x0C, 0x0E, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x0F, 0x0F,
0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x07,
0x0F, 0x0E, 0x0F, 0x03, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x0D, 0x0D, 0x0D, 0x0D, 0x01, 0x00, 0x00,
0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x0D, 0x0F, 0x0F, 0x07, 0x00, 0x00, 0x0F,
0x0F, 0x0F, 0x00, 0x00, 0x07, 0x0F, 0x0F, 0x0C, 0x0C, 0xC0, 0xC0, 0xC0, 0xE7, 0xFF, 0x7C, 0x1F,
0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
uint8_t NOKIA_ORIENTATION = 0;

/**
 * @name setPin
 * @param pin  instance of struct PIN_t, containing PORT and PIN
 * @return none
 * @brief sets an output pin to a logical 1
 */
void setPin(PIN_t pin)
{
  pin.Port->OUTSET = pin.Pin_bm;
}

/**
 * @name clrPin
 * @param pin  instance of struct PIN_t, containing PORT and PIN
 * @return none
 * @brief sets an output pin to a logical 0
 */
void clrPin(PIN_t pin)
{
  pin.Port->OUTCLR = pin.Pin_bm;
}

/**
 * @name outdirPin
 * @param pin  instance of struct PIN_t, containing PORT and PIN
 * @return none
 * @brief declares a pin to be an output
 */
void outdirPin(PIN_t pin)
{
  pin.Port->DIRSET = pin.Pin_bm;
}

/**
 * @name NOKIA_writeCommand
 * @param data uint8_t of data
 * @return none
 * @brief writes one byte in command-mode to the display
 */
void NOKIA_writeCommand (uint8_t command )
{
  clrPin(NOKIA_DC);     // set LCD into command mode
  uint8_t dummy = NOKIA_SPI->INTFLAGS;
  NOKIA_SPI->DATA = command;
  while ((NOKIA_SPI->INTFLAGS & SPI_IF_bm) == 0);
}

/**
 * @name NOKIA_writeData
 * @param data uint8_t of data
 * @return none
 * @brief writes one byte in data-mode to the display
 */
void NOKIA_writeData (uint8_t data )
{
  setPin(NOKIA_DC);     // set LCD into command mode
  uint8_t dummy = NOKIA_SPI->INTFLAGS;
  NOKIA_SPI->DATA = data;
  while ((NOKIA_SPI->INTFLAGS & SPI_IF_bm) == 0);
}

/**
 * @name NOKIA_gotoXY
 * @param x x-coordinate 0..83
 * @param y y-coordinate 0..47
 * @return none
 * @brief sets the displays write pointer - for partial writes (not used)
 */
void NOKIA_gotoXY ( uint8_t x, uint8_t y )
{
    NOKIA_writeCommand (0x80 | x);   //column
    NOKIA_writeCommand (0x40 | (y/8));   //row
}

/**
 * @name NOKIA_fillbuffer
 * @param value byte value to fill the display with
 * @return none
 * @brief fills the display buffer with a given bit pattern
 */
void NOKIA_fillbuffer(uint8_t value)
{
  memset(NOKIA_FRAMEBUFFER, value, NOKIA_SIZEX*NOKIA_SIZEY/8);
}

/**
 * @name NOKIA_clearbuffer
 * @param none
 * @return none
 * @brief clears the display buffer (no update to the display)
 */
void NOKIA_clearbuffer(void)
{
  NOKIA_fillbuffer(0);
}

/**
 * @name NOKIA_update
 * @param none
 * @return none
 * @brief updates the display with the current framebuffer
 */
void NOKIA_update (void)
{
  uint16_t i;

  NOKIA_gotoXY(0,0);      // start with (0,0) position
  switch (NOKIA_ORIENTATION)
  {
    case NOKIA_ORIENTATION_180:
        NOKIA_SPI->CTRLA |= SPI_DORD_bm; // LSB first
        for(i=0; i<(NOKIA_SIZEX*NOKIA_SIZEY/8); i++)
        {
        NOKIA_writeData(NOKIA_FRAMEBUFFER[NOKIA_SIZEX*NOKIA_SIZEY/8-i-1]);
        }
        NOKIA_SPI->CTRLA &= ~SPI_DORD_bm; // back to MSB first
        break;

    default:
        for(i=0; i<(NOKIA_SIZEX*NOKIA_SIZEY/8); i++)
        {
        NOKIA_writeData(NOKIA_FRAMEBUFFER[i]);
        }
        break;
  }
  NOKIA_gotoXY(0,0);      // bring the XY position back to (0,0)
}

/**
 * @name NOKIA_clear
 * @param none
 * @return none
 * @brief clears the display buffer and updates the display
 */
void NOKIA_clear ( void )
{
    NOKIA_clearbuffer();
    NOKIA_update();
}

/**
 * @name NOKIA_init
 * @param spi SPI-struct of the used SPI interface
 * @param rst_port PORT-struct of the RST pin
 * @param rst_pin pin number of the RST pin
 * @param dc_port PORT-struct of the DC pin
 * @param dc_pin pin number of the DC pin
 * @param vop  the contrast voltage parameter 0..127
 * @param orientation  orientation of the display (0: 0°, 1: 180°)
 * @return none
 * @brief initialize the NOKIA display, the SCK and MOSI pins need to be outputs
 */
void NOKIA_init (
  volatile SPI_t *spi,
  volatile PORT_t *rst_port, uint8_t rst_pin,
  volatile PORT_t *dc_port, uint8_t dc_pin,
  uint8_t vop,
  uint8_t orientation
)
{
  NOKIA_SPI = spi;
  NOKIA_SPI->CTRLA = SPI_MASTER_bm;
  NOKIA_SPI->CTRLA |= (0 << SPI_CLK2X_bp) | SPI_PRESC_DIV4_gc;
  NOKIA_SPI->CTRLB = SPI_MODE_0_gc | SPI_SSD_bm;
  NOKIA_SPI->CTRLA |= (0 << SPI_DORD_bp);  // MSB first
  NOKIA_SPI->CTRLA |= SPI_ENABLE_bm;
  uint8_t dummy = NOKIA_SPI->INTFLAGS;

  NOKIA_ORIENTATION = orientation;
  NOKIA_RST.Port = rst_port;
  NOKIA_RST.Pin_bm = 1 << rst_pin;
  outdirPin(NOKIA_RST);
  NOKIA_DC.Port = dc_port;
  NOKIA_DC.Pin_bm = 1 << dc_pin;
  outdirPin(NOKIA_DC);
  _delay_ms(100);

  clrPin(NOKIA_RST); // reset LCD
  _delay_ms(100);
  setPin(NOKIA_RST); // reset LCD

  NOKIA_writeCommand( 0x21 );  // LCD Extended Commands.
  NOKIA_writeCommand( 0x80 | vop );  // Set LCD Vop (Contrast).
  NOKIA_writeCommand( 0x04 );  // Set Temp coefficent.
  NOKIA_writeCommand( 0x13 );  // LCD bias mode 1:48.
  NOKIA_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
  NOKIA_writeCommand( 0x0c );  // LCD in normal mode.

  NOKIA_update();
}

/**
 * @name NOKIA_setVop
 * @param vop  the contrast voltage parameter 0..127
 * @return none
 * @brief sets the display's contrast voltage
 */
void NOKIA_setVop(uint8_t vop)
{
    NOKIA_writeCommand( 0x21 );  // LCD Extended Commands.
    NOKIA_writeCommand( 0x80 | vop );  // Set LCD Vop (Contrast).
    NOKIA_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
    NOKIA_writeCommand( 0x0c );  // LCD in normal mode.
}

/**
 * @name NOKIA_setpixel
 * @param x  x-coordinate 0..83
 * @param y  y-coordinate 0..47
 * @return none
 * @brief sets a single pixel at (x,y)
 */
void NOKIA_setpixel(uint8_t x, uint8_t y)
{
  if ((x < NOKIA_SIZEX) && (y < NOKIA_SIZEY))
  {
    NOKIA_FRAMEBUFFER[(uint16_t) x+NOKIA_SIZEX*(y/8)] |= (1 << (y % 8));
  }
}

/**
 * @name NOKIA_clearpixel
 * @param x  x-coordinate 0..83
 * @param y  y-coordinate 0..47
 * @return none
 * @brief clears a single pixel at (x,y)
 */
void NOKIA_clearpixel(uint8_t x, uint8_t y)
{
  if ((x < NOKIA_SIZEX) && (y < NOKIA_SIZEY))
  {
    NOKIA_FRAMEBUFFER[(uint16_t) x+NOKIA_SIZEX*(y/8)] &= ~(1 << (y % 8));
  }
}

/**
 * @name NOKIA_putchar
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param ch  character 0..255
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief puts a single character onto LCD
 */
void NOKIA_putchar(uint8_t x0, uint8_t y0, char ch, uint8_t attr)
{
  uint8_t yd, ym, i, fontbyte;
  uint16_t m;
  yd = y0/8;
  ym = y0%8;
  for (i=0; i<6; i++)
  {
    fontbyte = pgm_read_byte(&smallFont[(uint8_t)ch][i]);
    switch (attr)
    {
      case  0:
          break;
      case  1:
          fontbyte ^= 0xff;
          break;
      case  2:
          fontbyte |= 0b10000000;
          break;
    }

    if ((x0+i)<NOKIA_SIZEX)
    {
      m = (uint16_t) x0+i+NOKIA_SIZEX*(yd);
      NOKIA_FRAMEBUFFER[m] &= ~(0xff << ym);
      NOKIA_FRAMEBUFFER[m] |= (fontbyte << ym);
      if ((y0<(NOKIA_SIZEY-8)) && (ym != 0))
      {
        m = (uint16_t) x0+i+NOKIA_SIZEX*(yd+1);
        NOKIA_FRAMEBUFFER[m] &= ~(0xff >> (8-ym));
        NOKIA_FRAMEBUFFER[m] |= (fontbyte >> (8-ym));
      }
    }
  }
}

/**
 * @name NOKIA_puttinychar
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param ch  character 0..255
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief puts a single character from 4x6 font onto LCD
 */
void NOKIA_puttinychar(uint8_t x0, uint8_t y0, char ch, uint8_t attr)
{
  uint8_t i, fontbyte;
  uint16_t m;
  if (ch > 93)
  {
    ch = ch & 0b01011111;
  }
  for (i=0; i<3; i++)
  {
    fontbyte = pgm_read_byte(&tinyFont[(uint8_t)ch][i]);
    switch (attr)
    {
      case  0:
          break;
      case  1:
          fontbyte ^= 0xff;
          break;
      case  2:
          if (i==2)
          {
            fontbyte |= 0b00001111;
          }
          break;
    }
    uint8_t fh = (fontbyte & 0xf0) >> 4;
    for (uint8_t j=0; j<4; j++)
    {
      if (fh & 0b00001000)
      {
        NOKIA_setpixel(x0+j,y0+2*i);
      }
      else
      {
        NOKIA_clearpixel(x0+j,y0+2*i);
      }
      if (fontbyte & 0b00001000)
      {
        NOKIA_setpixel(x0+j,y0+1+2*i);
      }
      else
      {
        NOKIA_clearpixel(x0+j,y0+1+2*i);
      }
      fontbyte <<= 1;
      fh <<= 1;
    }
  }
}

/**
 * @name NOKIA_print
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string into the framebuffer
 */
void NOKIA_print(uint8_t x, uint8_t y, char *ch, uint8_t attr)
{
  while (*ch)
  {
    NOKIA_putchar(x, y, *ch, attr);
    ch++;
    x += 6;
  }
}

/**
 * @name NOKIA_print_P
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string in PROGMEM
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string from PROGMEM into the framebuffer
 */
void NOKIA_print_P(uint8_t x, uint8_t y, const char *ch,uint8_t attr)
{
  char c;
  while ((c = pgm_read_byte(ch)))
  {
    NOKIA_putchar(x, y, c, attr);
    ch++;
    x += 6;
  }
}

/**
 * @name NOKIA_printtiny
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string into the framebuffer using the 4x6 font
 */
void NOKIA_printtiny(uint8_t x, uint8_t y, char *ch, uint8_t attr)
{
  while (*ch)
  {
    NOKIA_puttinychar(x, y, *ch, attr);
    ch++;
    x += 4;
  }
}

/**
 * @name NOKIA_printtiny_P
 * @param x  x-coordinate upper-left corner 0..83
 * @param y  y-coordinate upper-left corner 0..47
 * @param *ch  pointer to null-terminated string in PROGMEM
 * @param attr  attribute (0-normal, 1-inverse, 2-underline)
 * @return none
 * @brief prints a character string from PROGMEM into the framebuffer using the 4x6 font
 */
void NOKIA_printtiny_P(uint8_t x, uint8_t y, const char *ch,uint8_t attr)
{
  char c;
  while ((c = pgm_read_byte(ch)))
  {
    NOKIA_puttinychar(x, y, c, attr);
    ch++;
    x += 4;
  }
}

/**
 * @name NOKIA_scroll
 * @param dy  y-shift -47..47
 * @return none
 * @brief shifts the contents of the framebuffer vertically
 */
void NOKIA_scroll(int8_t dy)
{
  int8_t y1;
  uint8_t  x, y, dy1, dy8, b1, b2;
  if (dy>0)
  {
    dy8 = dy/8;
    dy1 = dy%8;
    for (x=0; x<NOKIA_SIZEX; x++)
    {
      for (y=0; y<(NOKIA_SIZEY/8); y++)
      {
        y1=y+dy8;
        if (y1<(NOKIA_SIZEY/8))
        {
          b1 = NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*y1];
        }
        else
        {
          b1=0;
        }
        if ((y1+1)<(NOKIA_SIZEY/8))
        {
          b2 = NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*(y1+1)];
        }
        else
        {
          b2=0;
        }
        NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*(y)] = (b1 >> dy1) | (b2 << (8-dy1));
      }
    }
  }
  else
  {
    dy8 = abs(dy)/8;
    dy1 = abs(dy)%8;
    for (x=0; x<NOKIA_SIZEX; x++)
    {
      for (y=0; y<(NOKIA_SIZEY/8); y++)
      {
        y1=(NOKIA_SIZEY/8)-y-dy8-1;
        if (y1>=0)
        {
          b1 = NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*y1];
        }
        else
        {
          b1=0;
        }
        if ((y1-1)>=0)
        {
          b2 = NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*(y1-1)];
        }
        else
        {
          b2=0;
        }
        NOKIA_FRAMEBUFFER[x + NOKIA_SIZEX*((NOKIA_SIZEY/8)-y-1)] = (b1 << dy1) | (b2 >> (8-dy1));
      }
    }
  }
}
