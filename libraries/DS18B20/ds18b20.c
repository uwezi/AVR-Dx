#include <ds18b20.h>

volatile PORT_t *DS_PORT;
uint8_t  DS_PIN_bm;
uint64_t DS_addresses[DS_MAX_DEVICES];
uint16_t DS_devcount = 0;

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_init
  Description  :  sets port and pin for the sensor
  Argument(s)  :  None.
  Return value :  0 - if DS18B20 detected
--------------------------------------------------------------------------------------------------*/
void DS18B20_init(volatile PORT_t *ds_port, uint8_t pin)
{
    DS_PORT   = ds_port;
    DS_PIN_bm = 1 << pin;
    DS_PORT->PINCONFIG = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;
    DS_PORT->PINCTRLUPD = DS_PIN_bm;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_set
  Description  :  sets port pin to 0 and output
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void DS18B20_set()
{
    DS_PORT->DIRSET = DS_PIN_bm;
    DS_PORT->OUTCLR = DS_PIN_bm;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_release
  Description  :  sets port pin input with pullup
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void DS18B20_release()
{
    DS_PORT->DIRCLR = DS_PIN_bm;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_get
  Description  :  reads pin status
  Argument(s)  :  None.
  Return value :  0/1
--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_get()
{
    return (DS_PORT->IN & DS_PIN_bm) ? 1 : 0;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_reset
  Description  :  Resets the DS18B20
  Argument(s)  :  None.
  Return value :  0 - if DS18B20 detected
--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_reset (void)
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

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_writeBit
  Description  :  writes a single bit
  Argument(s)  :  0/1.
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_writeBit (uint8_t bit)
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

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_write
  Description  :  writes a byte
  Argument(s)  :  byte to write
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_write(uint8_t byte)
{  // LSB first
  uint8_t i;
  for (i=8; i>0; i--)
  {
    DS18B20_writeBit(byte & 0b00000001);
    byte >>= 1;
  }
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_readBit
  Description  :  read a single bit
  Argument(s)  :  None.
  Return value :  0/1
--------------------------------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_read
  Description  :  reads a byte
  Argument(s)  :  None.
  Return value :  byte

--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_read(void)
{  // LSB first
  uint8_t i, result=0;

  for (i=8; i>0; i--)
  {
    result >>= 1;
    result |= (DS18B20_readBit() & 0b00000001) << 7;
  }
  return result;
}


/************************************************************
*                                                           *
* 1-wire ROM Search routine                                 *
*                                                           *
* This is an alternative to the published routine. It is    *
* not faster than the publshed routine (the number of       *
* passes is the same), but it may be easier to implement in *
* a state machine. It was developed for use in VHDL but is  *
* written here in C for general readability.                *
*                                                           *
* The variables 'path', 'next', and 'pos' need to have as   *
* many bits as there are possible collisions on one pass.   *
* If there are N devices, this number of bits is at least   *
* log_2(N) and not more than N-1 (and not more than 56).    *
*                                                           *
* This routine does not admit the possibility that the two  *
* bits read from the bus are both 1. This would only happen *
* in the event of an error after device presence has been   *
* obtained.                                                 *
*                                                           *
* The variable 'path' contains sufficient information to    *
* conduct one pass. The results of previous passes are not  *
* required to conduct the next pass.                        *
*                                                           *
* The sentences in parentheses are interface specific.      *
*                                                           *
* Robert Jensen                                             *
* robertjensen@verizon.net                                  *
* September 14, 2010                                        *
*                                                           *
************************************************************/
uint16_t DS18B20_scanBus (void)
{
  uint64_t addr,path,next,pos;              /* decision markers */
  int16_t count, numdev=0;                             /* bit count */
  uint8_t bit,chk;                          /* bit values */

  path=0;                                   /* initial path to follow */
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
  DS_devcount = numdev;
  return numdev;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_select
  Description  :  select a single DS18B20 on the bus by its address
  Argument(s)  :  address 64bit
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_select(uint64_t address)
{
  uint8_t i;
  DS18B20_write(DS18B20_CMD_MATCHROM);
  for (i=0; i<64; i++)
  {
    DS18B20_writeBit((address >> i) & 0b01);
  }
}
