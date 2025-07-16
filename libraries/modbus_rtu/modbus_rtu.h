/**
 * @file modbus_rtu.h
 * @brief MODBUS/RTU library for AVR-Dx series
 *
 * @author Uwe Zimmermann
 *
 * The library work is licensed under a MIT license.\n
 * See https://github.com/uwezi/AVR-Dx
 *
 * A basic MODBUS/RTU implementation which can be attached to any USART module
 * on the AVR-Dx series microcontrollers.
 *
 * Supported MODBUS functions are
 * - 0x03 read holding register
 * - 0x04 read input register - same register block as 0x03
 * - 0x06 write single holding register
 * - 0x16 write multiple holding registers
 *
 * Uses TCB1, TCB2 and EVSYS.CHANNEL0 for timeout control
 *
 *
 * ChangeLog:
 * --------
 * * 2025-07-14 created.
 */

#ifndef modbus_rtu_h
#define modbus_rtu_h

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

/**
 * @brief hardware parameters of the UART module to be used
 * @note uses the UART in RS-485 one-wire mode
**/
#define UART             USART0
#define UART_INTVEC      USART0_RXC_vect
#define UART_ROUTEREG    PORTMUX_USARTROUTEA
#define UART_PINROUTE_gm PORTMUX_USART0_gm
#define UART_PINROUTE_gc PORTMUX_USART0_ALT1_gc
#define UART_XDIRSET     PORTA.DIRSET = PIN7_bm;
#define UART_TXPINPULLUP PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
#define BAUD_RATE        9600
#define UARTTIMEOUT      (uint32_t)((float)1000000.0/((float)BAUD_RATE/11)*3.5)
#define UART_BAUD_CALC(BAUD_RATE) \
    ((float) ( F_CPU * 64 /  ( 16 * (float)BAUD_RATE )) + 0.5 )

/**
 * @brief array for the MODBUS holding registers, element at index 0 is ignored
 * @note shared with the application code
**/
#define mbHOLDINGSIZE 1000
extern volatile uint16_t mbHolding[mbHOLDINGSIZE+1];

/**
 * @brief MODBUS/RTU address of the server, shared with the application R/W
**/
extern volatile uint8_t mbAdress;

/**
 * \name
 * @param address Modbus/RTU address, 1..255
 * @return none
 * @brief initialize the Modbus/RTU server
 */
void MODBUS_init(uint8_t address);

#endif