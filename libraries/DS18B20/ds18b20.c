/**
 * @file ds18b20.c
 * @brief 1-wire temperature sensor library for AVR-Dx series
 *
 * @author Uwe Zimmermann
 *
 * The library work is licensed under a MIT license.\n
 * See https://github.com/uwezi/AVR-Dx
 *
 * See the datasheet of the DS18B20 for details
 *
 * ChangeLog:
 * --------
 * * 2025-07-11 created.
 */

#include <ds18b20.h>

/**
 * @brief globals
 */
volatile PORT_t *DS_PORT;
uint8_t  DS_PIN_bm;
uint64_t DS_addresses[DS_MAX_DEVICES];
uint16_t DS_devcount = 0;

/**
 * @name DS18B20_init()
 * @param ds_port - PORT-module for the 1-wire devices
 * @param pin - PIN number for the 1-wire devices 0..7
 * @return none
 * @brief initialize PORT for 1-wire communication
 */
void DS18B20_init(volatile PORT_t *ds_port, uint8_t pin)
{
    DS_PORT   = ds_port;
    DS_PIN_bm = 1 << pin;
    DS_PORT->PINCONFIG = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;
    DS_PORT->PINCTRLUPD = DS_PIN_bm;
}

/**
 * @name DS18B20_set()
 * @brief set the pin to 0 and declare as output
 * @note internal use
 */
void DS18B20_set(void)
{
    DS_PORT->DIRSET = DS_PIN_bm;
    DS_PORT->OUTCLR = DS_PIN_bm;
}

/**
 * @name DS18B20_release()
 * @brief release the port pin, keep pull-up active
 * @note internal use
 */
void DS18B20_release(void)
{
    DS_PORT->DIRCLR = DS_PIN_bm;
}

/**
 * @name DS18B20_get()
 * @param none
 * @return uint8_t state of the pin 1/0
 * @brief reads the state of the 1-wire line, receiving the answer from the device
 * @note internal use
 */
uint8_t DS18B20_get(void)
{
    return (DS_PORT->IN & DS_PIN_bm) ? 1 : 0;
}

/**
 * @name DS18B20_reset()
 * @return uint8_t returns the state of the 1-wire line after a bus-reset
 * @brief resets the 1-wire bus devices by pulling the line low for 480 µs
 * @note internal use
 */
uint8_t DS18B20_reset(void)
{
  uint8_t result;

  DS18B20_set();
  _delay_us(480);
  DS18B20_release();
  _delay_us(60);
  result = DS18B20_get();
  _delay_us(420);
  return result;
}

/**
 * @name DS18B20_writeBit()
 * @param bit - the bit to write 1/0
 * @return none
 * @brief writes a bit on the 1-wire bus
 * @note internal use
 */
void DS18B20_writeBit(uint8_t bit)
{
  DS18B20_set();
  _delay_us(1);
  if (bit > 0)
  {
    DS18B20_release();
  }
  _delay_us(59);
  DS18B20_release();
  _delay_us(1);
}

/**
 * @name DS18B20_write()
 * @param byte - data to be written to the 1-wire devices
 * @return none
 * @brief writes one byte of data on the 1-wire bus
 */
void DS18B20_write(uint8_t byte)
{  // LSB first
  uint8_t i;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for (i=8; i>0; i--)
    {
      DS18B20_writeBit(byte & 0b00000001);
      byte >>= 1;
    }
  }
}

/**
 * @name DS18B20_write_config()
 * @param THIGH high-threshold for the alarm
 * @param TLOW low-threshold for the alarm
 * @param CONFIG configuration byte for the DS18B20
 * @return none
 * @brief writes three bytes to the configuration part of the scratchpad
 */
void DS18B20_write_config(int8_t THIGH, int8_t TLOW, uint8_t CONFIG)
{
    DS18B20_write(DS18B20_CMD_WSCRATCHPAD);
    DS18B20_write(THIGH);
    DS18B20_write(TLOW);
    DS18B20_write(CONFIG);
}

/**
 * @name DS18B20_readBit()
 * @return uint8_t - value of the bit 1/0
 * @brief triggers the reading of a single bit from the device
 * @note internal use
 */
uint8_t DS18B20_readBit (void)
{
  uint8_t result;

  DS18B20_set();
  _delay_us(1);
  DS18B20_release();
  _delay_us(15);
  result = DS18B20_get();
  DS18B20_release();
  _delay_us(45);
  return result;
}

/**
 * @name DS18B20_read()
 * @return uint8_t - value returned by the device, 1 byte
 * @brief triggers the reading of a single byte from the device
 */
uint8_t DS18B20_read(void)
{  // LSB first
  uint8_t i, result=0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for (i=8; i>0; i--)
    {
      result >>= 1;
      result |= (DS18B20_readBit() & 0b00000001) << 7;
    }
  }
  return result;
}


/**
 * @name DS18B20_scanBus()
 * @return uint16_t - number of discovered devices on the bus
 * @return DS_addresses[] - fills the global array of found addresses
 * @brief scans the bus for the IDs of the attached devices
 * 1-wire ROM Search routine
 *
 * This is an alternative to the published routine. It is
 * not faster than the publshed routine (the number of
 * passes is the same), but it may be easier to implement in
 * a state machine. It was developed for use in VHDL but is
 * written here in C for general readability.
 *
 * The variables 'path', 'next', and 'pos' need to have as
 * many bits as there are possible collisions on one pass.
 * If there are N devices, this number of bits is at least
 * log_2(N) and not more than N-1 (and not more than 56).
 *
 * This routine does not admit the possibility that the two
 * bits read from the bus are both 1. This would only happen
 * in the event of an error after device presence has been
 * obtained.
 *
 * The variable 'path' contains sufficient information to
 * conduct one pass. The results of previous passes are not
 * required to conduct the next pass.
 *
 * The sentences in parentheses are interface specific.                                                                 *
 * Robert Jensen
 * robertjensen@verizon.net
 * September 14, 2010
 */
uint16_t DS18B20_scanBus(void)
{
  uint64_t addr,path,next,pos;              /* decision markers */
  int16_t count, numdev=0;                             /* bit count */
  uint8_t bit,chk;                          /* bit values */

  path=0;                                   /* initial path to follow */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    do
    {                                         /* each ROM search pass */
      DS18B20_reset();
      DS18B20_write(DS18B20_CMD_SEARCHROM);
      addr = 0;
      next=0;                                 /* next path to follow */
      pos=1;                                  /* path bit pointer */
      count=0;                                /* count the bits */
      do
      {                                       /* each bit of the ROM value */
        bit = DS18B20_readBit();
        chk = DS18B20_readBit();
          if (!bit && !chk)
          {                                   /* collision, both are zero */
            if (pos & path)
            {
              bit=1;             /* if we've been here before */
            }
            else
            {
              next=(path&(pos-1))|pos;   /* else, new branch for next */
            }
            pos<<=1;
          }
          DS18B20_writeBit(bit);
          addr |= (uint64_t)bit << count;
          count++;
      } while (count<64);

      if (numdev < DS_MAX_DEVICES)
      {
        DS_addresses[numdev] = addr;
        numdev++;
      }
      _delay_ms(1);
      path=next;
    } while(path);
  } // atomic block
  DS_devcount = numdev;
  return numdev;
}

/**
 * @name DS18B20_select()
 * @return none
 * @param address 64-bit ID address of the desired device
 * @brief selects one 1-wire device on the bus for subsequent activities
 */
void DS18B20_select(uint64_t address)
{
  uint8_t i;
  DS18B20_write(DS18B20_CMD_MATCHROM);
  for (i=0; i<64; i++)
  {
    DS18B20_writeBit((address >> i) & 0b01);
  }
}
