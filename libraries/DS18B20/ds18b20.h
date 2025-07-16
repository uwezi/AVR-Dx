/**
 * @file ds18b20.h
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

#ifndef ds18b20_h
#define ds18b20_h

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>

/**
 * list of found sensors on the 1-wire bus after a DS18B20_scanBus()
 */
#define DS_MAX_DEVICES 10
extern uint64_t DS_addresses[DS_MAX_DEVICES];
extern uint16_t DS_devcount;

/**
 * command constants for the DS18B20 sensor
 */
#define DS18B20_CMD_CONVERTTEMP 0x44   //!< start a temperature measurement
#define DS18B20_CMD_RSCRATCHPAD 0xbe   //!< read from scratchpad
#define DS18B20_CMD_WSCRATCHPAD 0x4e   //!< write to scratchpad
#define DS18B20_CMD_CPYSCRATCHPAD 0x48 //!< copy scratchpad to EEPROM
#define DS18B20_CMD_RECEEPROM 0xb8     //!< recall values from EEPROM
#define DS18B20_CMD_RPWRSUPPLY 0xb4    //!< check for parasite power
#define DS18B20_CMD_SEARCHROM 0xf0     //!< determines 1-wire bus addresses
#define DS18B20_CMD_READROM 0x33       //!< if only one slave: read bus address
#define DS18B20_CMD_MATCHROM 0x55      //!< select a single slave device
#define DS18B20_CMD_SKIPROM 0xcc       //!< select all devices on the bus
#define DS18B20_CMD_ALARMSEARCH 0xec   //!< like SEARCHROM but only devices with active alarm will react

/**
 * configuration constants for temperature resolution
 */
#define DS18B20_CFG_9BIT  0b00 << 5 //!< 1/2°C resolution, 9 bits of result, 94 ms
#define DS18B20_CFG_10BIT 0b01 << 5 //!< 1/4°C resolution, 10 bits of result, 188 ms
#define DS18B20_CFG_11BIT 0b10 << 5 //!< 1/8°C resolution, 11 bits of result, 375 ms
#define DS18B20_CFG_12BIT 0b11 << 5 //!< 1/16°C resolution, 12 bits of result, 750 ms


/**
 * @name DS18B20_init()
 * @param ds_port - PORT-module for the 1-wire devices
 * @param pin - PIN number for the 1-wire devices 0..7
 * @return none
 * @brief initialize PORT for 1-wire communication
 */
void DS18B20_init(volatile PORT_t *ds_port, uint8_t pin);

/**
 * @name DS18B20_set()
 * @brief set the pin to 0 and declare as output
 * @note internal use
 */
void DS18B20_set(void);

/**
 * @name DS18B20_release()
 * @brief release the port pin, keep pull-up active
 * @note internal use
 */
void DS18B20_release(void);

/**
 * @name DS18B20_get()
 * @param none
 * @return uint8_t state of the pin 1/0
 * @brief reads the state of the 1-wire line, receiving the answer from the device
 * @note internal use
 */
uint8_t DS18B20_get(void);

/**
 * @name DS18B20_reset()
 * @return uint8_t returns the state of the 1-wire line after a bus-reset
 * @brief resets the 1-wire bus devices by pulling the line low for 480 µs
 * @note internal use
 */
uint8_t DS18B20_reset(void);

/**
 * @name DS18B20_writeBit()
 * @param bit - the bit to write 1/0
 * @return none
 * @brief writes a bit on the 1-wire bus
 * @note internal use
 */
void DS18B20_writeBit(uint8_t bit);

/**
 * @name DS18B20_write()
 * @param byte - data to be written to the 1-wire devices
 * @return none
 * @brief writes one byte of data on the 1-wire bus
 */
void DS18B20_write(uint8_t byte);

/**
 * @name DS18B20_write_config()
 * @param THIGH high-threshold for the alarm
 * @param TLOW low-threshold for the alarm
 * @param CONFIG configuration byte for the DS18B20
 * @return none
 * @brief writes three bytes to the configuration part of the scratchpad
 */
void DS18B20_write_config(int8_t THIGH, int8_t TLOW, uint8_t CONFIG);

/**
 * @name DS18B20_readBit()
 * @return uint8_t - value of the bit 1/0
 * @brief triggers the reading of a single bit from the device
 * @note internal use
 */
uint8_t DS18B20_readBit(void);

/**
 * @name DS18B20_read()
 * @return uint8_t - value returned by the device, 1 byte
 * @brief triggers the reading of a single byte from the device
 */
uint8_t DS18B20_read(void);

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
uint16_t DS18B20_scanBus(void);

/**
 * @name DS18B20_select()
 * @return none
 * @param address 64-bit ID address of the desired device
 * @brief selects one 1-wire device on the bus for subsequent activities
 */
void DS18B20_select(uint64_t address);

#endif
