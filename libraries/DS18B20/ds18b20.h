/// \file ds18b20.h
/// \brief DS18B20 library for AVR-Dx series
///
/// \author Uwe Zimmermann
///
/// The library work is licensed under a MIT license.\n
/// See https://github.com/uwezi/AVR-Dx
///
/// ChangeLog:
/// --------
/// * 2025-07-11 created.

#ifndef ds18b20_h
#define ds18b20_h

#include <avr/io.h>
#include <util/delay.h>

#define DS_MAX_DEVICES 10
extern uint64_t DS_addresses[DS_MAX_DEVICES];
extern uint16_t DS_devcount;

/*--------------------------------------------------------------------------------------------------
  define constants for the DS18B20 commands
--------------------------------------------------------------------------------------------------*/

#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_init
  Description  :  sets port and pin for the sensor
  Argument(s)  :  None.
  Return value :  0 - if DS18B20 detected
--------------------------------------------------------------------------------------------------*/
void DS18B20_init(volatile PORT_t *ds_port, uint8_t pin);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_reset
  Description  :  Resets the DS18B20
  Argument(s)  :  None.
  Return value :  0 - if DS18B20 detected
--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_reset (void);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_writeBit
  Description  :  writes a single bit
  Argument(s)  :  0/1.
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_writeBit (uint8_t bit);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_write
  Description  :  writes a byte
  Argument(s)  :  byte to write
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_write(uint8_t byte);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_readBit
  Description  :  read a single bit
  Argument(s)  :  None.
  Return value :  0/1
--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_readBit (void);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_read
  Description  :  reads a byte
  Argument(s)  :  None.
  Return value :  byte

--------------------------------------------------------------------------------------------------*/
uint8_t DS18B20_read(void);

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
uint16_t DS18B20_scanBus (void);

/*--------------------------------------------------------------------------------------------------
  Name         :  DS18B20_select
  Description  :  select a single DS18B20 on the bus by its address
  Argument(s)  :  address 64bit
  Return value :  none
--------------------------------------------------------------------------------------------------*/
void DS18B20_select(uint64_t address);

#endif
