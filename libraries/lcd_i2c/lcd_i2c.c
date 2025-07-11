/// \file i2c_lcd.c
/// \brief LiquidCrystal library with PCF8574 I2C adapter.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2019 by Matthias Hertel.
///
/// ChangeLog see: i2c_lcd.h

#include "lcd_i2c.h"

#define F_SCL 100000
#define T_RISE 0.3
#define TWI_BAUD(F_SCL, T_RISE) ((((((float)(F_CPU) / (float)(F_SCL))) - 10 - ((float)(F_CPU) * (T_RISE) / 1000000))) / 2)
volatile TWI_t *LCD_twiport;

enum
{
  I2C_INIT = 0,
  I2C_ACKED,
  I2C_NACKED,
  I2C_READY,
  I2C_ERROR
};

void LCD_init(uint8_t cols, uint8_t lines, TWI_t *twi,
              uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t e,
              uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight)
{
  LCD_i2cAddr = i2cAddr << 1;
  LCD_backlight = 0;

  LCD_entrymode = 0x02; // like Initializing by Internal Reset Circuit
  LCD_displaycontrol = 0x04;

  LCD_rs_mask = 0x01 << rs;
  if (rw != 255)
    LCD_rw_mask = 0x01 << rw;
  else
    LCD_rw_mask = 0;
  LCD_enable_mask = 0x01 << e;
  LCD_data_mask[0] = 0x01 << d4;
  LCD_data_mask[1] = 0x01 << d5;
  LCD_data_mask[2] = 0x01 << d6;
  LCD_data_mask[3] = 0x01 << d7;

  if (backlight != 255)
    LCD_backlight_mask = 0x01 << backlight;
  else
    LCD_backlight_mask = 0;

  LCD_twiport = twi; // Grab which port the user wants us to use

  LCD_cols = cols;
  LCD_lines = lines;

  uint8_t functionFlags = 0;
  if (lines > 1)
  {
    functionFlags |= 0x08;
  }

  LCD_row_offsets[0] = 0x00;
  LCD_row_offsets[1] = 0x40;
  LCD_row_offsets[2] = 0x00 + cols;
  LCD_row_offsets[3] = 0x40 + cols;

  // initializing the display
  LCD_twiport->CTRLA = 0x00;
  LCD_twiport->MBAUD = (uint8_t)TWI_BAUD(F_SCL, 0.1);

  /* Enable TWI */
  LCD_twiport->MCTRLA = TWI_ENABLE_bm;

  /* Host Address */
  LCD_twiport->MADDR = 0x0;

  /* ACKACT ACK; FLUSH disabled; MCMD NOACT */
  LCD_twiport->MCTRLB = 0x0;

  /* Host Data */
  LCD_twiport->MDATA = 0x0;

  LCD_twiport->MCTRLB |= TWI_FLUSH_bm;
  LCD_twiport->MSTATUS |= TWI_BUSSTATE_IDLE_gc;

  _delay_us(50000);

  // after reset the mode is this
  LCD_displaycontrol = 0x04;
  LCD_entrymode = 0x02;

  // sequence to reset. see "Initializing by Instruction" in datasheet

  LCD_sendNibble(0x03, 0);
  _delay_us(4500);
  LCD_sendNibble(0x03, 0);
  _delay_us(200);
  LCD_sendNibble(0x03, 0);
  _delay_us(200);
  LCD_sendNibble(0x02, 0); // finally, set to 4-bit interface

  // Instruction: Function set = 0x20
  LCD_send(0x20 | functionFlags, 0);

  LCD_display();
  LCD_clear();
  LCD_leftToRight();

} // init()

void LCD_clear()
{
  // Instruction: Clear display = 0x01
  LCD_send(0x01, 0);
  _delay_us(1600); // this command takes 1.5ms!
} // clear()

void LCD_home()
{
  // Instruction: Return home = 0x02
  LCD_send(0x02, 0);
  _delay_us(1600); // this command takes 1.5ms!
} // home()

/// Set the cursor to a new position.
void LCD_setCursor(uint8_t col, uint8_t row)
{
  // check boundaries
  if ((col < LCD_cols) && (row < LCD_lines))
  {
    // Instruction: Set DDRAM address = 0x80
    LCD_send(0x80 | (LCD_row_offsets[row] + col), 0);
  }
} // setCursor()

// Turn the display on/off (quickly)
void LCD_noDisplay()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol &= ~0x04; // display
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // noDisplay()

void LCD_display()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol |= 0x04; // display
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // display()

// Turns the underline cursor on/off
void LCD_cursor()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol |= 0x02; // cursor
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // cursor()

void LCD_noCursor()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol &= ~0x02; // cursor
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // noCursor()

// Turn on and off the blinking cursor
void LCD_blink()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol |= 0x01; // blink
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // blink()

void LCD_noBlink()
{
  // Instruction: Display on/off control = 0x08
  LCD_displaycontrol &= ~0x01; // blink
  LCD_send(0x08 | LCD_displaycontrol, 0);
} // noBlink()

// These commands scroll the display without changing the RAM
void LCD_scrollDisplayLeft(void)
{
  // Instruction: Cursor or display shift = 0x10
  // shift: 0x08, left: 0x00
  LCD_send(0x10 | 0x08 | 0x00, 0);
} // scrollDisplayLeft()

void LCD_scrollDisplayRight(void)
{
  // Instruction: Cursor or display shift = 0x10
  // shift: 0x08, right: 0x04
  LCD_send(0x10 | 0x08 | 0x04, 0);
} // scrollDisplayRight()

// == controlling the entrymode

// This is for text that flows Left to Right
void LCD_leftToRight(void)
{
  // Instruction: Entry mode set, set increment/decrement =0x02
  LCD_entrymode |= 0x02;
  LCD_send(0x04 | LCD_entrymode, 0);
} // leftToRight()

// This is for text that flows Right to Left
void LCD_rightToLeft(void)
{
  // Instruction: Entry mode set, clear increment/decrement =0x02
  LCD_entrymode &= ~0x02;
  LCD_send(0x04 | LCD_entrymode, 0);
} // rightToLeft()

// This will 'right justify' text from the cursor
void LCD_autoscroll(void)
{
  // Instruction: Entry mode set, set shift S=0x01
  LCD_entrymode |= 0x01;
  LCD_send(0x04 | LCD_entrymode, 0);
} // autoscroll()

// This will 'left justify' text from the cursor
void LCD_noAutoscroll(void)
{
  // Instruction: Entry mode set, clear shift S=0x01
  LCD_entrymode &= ~0x01;
  LCD_send(0x04 | LCD_entrymode, 0);
} // noAutoscroll()

/// Setting the brightness of the background display light.
/// The backlight can be switched on and off.
/// The current brightness is stored in the private _backlight variable to have it available for further data transfers.
void LCD_setBacklight(uint8_t brightness)
{
  LCD_backlight = brightness;
  // send no data but set the background-pin right;
  if (LCD_backlight)
  {
    LCD_write2Wire(LCD_backlight_mask, 1, 0);
  }
  else
  {
    LCD_write2Wire(LCD_backlight_mask, 1, 0);
  }
} // setBacklight()

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCD_createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  // Set CGRAM address
  LCD_send(0x40 | (location << 3), 0);
  for (uint8_t i = 0; i < 8; i++)
  {
    LCD_write(charmap[i]);
  }
} // createChar()

// Allows us to fill the first 8 CGRAM locations
// with custom characters stored in PROGMEM
void LCD_createChar_P(uint8_t location, const uint8_t *charmap)
{
  PGM_P p = (PGM_P)(charmap);
  location &= 0x7; // we only have 8 locations 0-7
  LCD_send(0x40 | (location << 3), 0);
  for (int i = 0; i < 8; i++)
  {
    uint8_t c = pgm_read_byte(p++);
    LCD_write(c);
  }
} // createChar_P()

/* The write function is needed for derivation from the Print class. */
inline size_t LCD_write(uint8_t ch)
{
  LCD_send(ch, 1);
  return 1; // assume success
} // write()

void LCD_print(char *ch)
{
  while (*ch != 0)
  {
    LCD_send(*ch, 1);
    ch++;
  }
}

static uint8_t i2c_0_WaitW(void)
{
  uint8_t state = I2C_INIT;
  do
  {
    if (LCD_twiport->MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
    {
      if (!(LCD_twiport->MSTATUS & TWI_RXACK_bm))
      {
        /* client responded with ack - TWI goes to M1 state */
        state = I2C_ACKED;
      }
      else
      {
        /* address sent but no ack received - TWI goes to M3 state */
        state = I2C_NACKED;
      }
    }
    else if (LCD_twiport->MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
    {
      /* get here only in case of bus error or arbitration lost - M4 state */
      state = I2C_ERROR;
    }
  } while (!state);
  return state;
}

static uint8_t i2c_0_WaitR(void)
{
  uint8_t state = I2C_INIT;

  do
  {
    if (LCD_twiport->MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
    {
      state = I2C_READY;
    }
    else if (LCD_twiport->MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
    {
      /* get here only in case of bus error or arbitration lost - M4 state */
      state = I2C_ERROR;
    }
  } while (!state);

  return state;
}

// write either command or data
uint8_t LCD_send(uint8_t value, uint8_t isData)
{
  // An I2C transmission has a significant overhead of ~10+1 I2C clock
  // cycles. We consequently only perform it only once per _send().

  uint8_t retVal = (uint8_t)-1;

  /* start transmitting the client address */
  LCD_twiport->MADDR = LCD_i2cAddr & ~0x01;
  if (i2c_0_WaitW() != I2C_ACKED)
    return retVal;

  retVal = 0;

  retVal = LCD_writeNibble((value >> 4 & 0x0F), isData);
  // write low 4 bits
  retVal = LCD_writeNibble((value & 0x0F), isData);
  LCD_twiport->MCTRLB = TWI_MCMD_STOP_gc;

} // _send()

// write a nibble / halfByte with handshake
uint8_t LCD_writeNibble(uint8_t halfByte, uint8_t isData)
{
  // map the data to the given pin connections
  uint8_t data = isData ? LCD_rs_mask : 0;
  // _rw_mask is not used here.
  if (LCD_backlight > 0)
    data |= LCD_backlight_mask;

  // allow for arbitrary pin configuration
  if (halfByte & 0x01)
    data |= LCD_data_mask[0];
  if (halfByte & 0x02)
    data |= LCD_data_mask[1];
  if (halfByte & 0x04)
    data |= LCD_data_mask[2];
  if (halfByte & 0x08)
    data |= LCD_data_mask[3];

  // Note that the specified speed of the PCF8574 chip is 100KHz.
  // Transmitting a single byte takes 9 clock ticks at 100kHz -> 90us.
  // The 37us delay is only necessary after sending the second nibble.
  // But in that case we have to restart the transfer using additional
  // >10 clock cycles. Hence, no additional delays are necessary even
  // when the I2C bus is operated beyond the chip's spec in fast mode
  // at 400 kHz.

  LCD_twiport->MDATA = data | LCD_enable_mask;
  if (i2c_0_WaitW() != I2C_ACKED)
  {
    return 1;
  }

  LCD_twiport->MDATA = data;
  if (i2c_0_WaitW() != I2C_ACKED)
  {
    return 1;
  }

} // _writeNibble

// write a nibble / halfByte with handshake
void LCD_sendNibble(uint8_t halfByte, uint8_t isData)
{
  uint8_t retVal = (uint8_t)-1;

  /* start transmitting the client address */
  LCD_twiport->MADDR = LCD_i2cAddr & ~0x01;
  if (i2c_0_WaitW() != I2C_ACKED)
    return retVal;

  retVal = 0;

  LCD_writeNibble(halfByte, isData);

  LCD_twiport->MSTATUS |= TWI_BUSSTATE_IDLE_gc;
} // _sendNibble

// private function to change the PCF8574 pins to the given value
void LCD_write2Wire(uint8_t data, uint8_t isData, uint8_t enable)
{
  if (isData)
    data |= LCD_rs_mask;
  // _rw_mask is not used here.
  if (enable)
    data |= LCD_enable_mask;
  if (LCD_backlight > 0)
    data |= LCD_backlight_mask;

  uint8_t retVal = (uint8_t)-1;

  /* start transmitting the client address */
  LCD_twiport->MADDR = LCD_i2cAddr & ~0x01;
  if (i2c_0_WaitW() != I2C_ACKED)
    return retVal;

  retVal = 0;

  LCD_twiport->MDATA = data;
  if (i2c_0_WaitW() != I2C_ACKED)
  {
    return 1;
  }

  LCD_twiport->MSTATUS |= TWI_BUSSTATE_IDLE_gc;
} // write2Wire

// The End.