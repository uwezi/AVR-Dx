/**
 * @file modbus_rtu.c
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

 #include <modbus_rtu.h>

/**
 * @brief MODBUS/RTU address of the server, shared with the application R/W
 */
volatile uint8_t mbAdress=0;

/**
 * @brief array for the MODBUS holding registers, element at index 0 is ignored
 * @note shared with the application code
 */
volatile uint16_t mbHolding[mbHOLDINGSIZE+1];

/**
 * @brief buffer for receiving and sending MODBUS/RTU messages
 * @note internal use only
 */
#define mbBUFFSIZE 256
volatile uint8_t mbBuffer[mbBUFFSIZE];
volatile uint16_t mbBufferPtr = 0;

/**
 * @brief handling of 16 bit parameters in MODBUS/RTU messages
 * @note internal use only
 */
#define MODBUS16BIT( BUFFER, INDEX ) ((BUFFER[INDEX]<<8) + BUFFER[INDEX+1])

/**
 * @brief static CRC table
 * @note borrowed from https://github.com/LacobusVentura/MODBUS-CRC16/
 */
static const PROGMEM uint16_t mb_crctable[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};

/**
 * Modbus_CRC16
 * @param *frame  pointer to buffer
 * @param framesize number of bytes in the buffer
 * @return crc -  returns 0 if buffer includes correct crc at the end
 * @brief calculates the CRC16 checksum over the MODBUS buffer
 */
uint16_t Modbus_CRC16(uint8_t *frame, uint16_t framesize)
{

    uint8_t xor = 0;
    uint16_t crc = 0xFFFF;

    while (framesize--)
    {
        xor = (*frame++) ^ crc;
        crc >>= 8;
        crc ^= pgm_read_word(&mb_crctable[xor]);
    }

    return crc;
}

/**
 * @param none
 * @brief enables and resets the timout timer
 */
static inline void MODBUS_Timout_Enable(void)
{
    TCB2.CNT = 0;
    TCB1.CNT = 0;
    TCB2.CTRLA |= TCB_ENABLE_bm;
}

/**
 * @param none
 * @brief resets the timeout timer -> calls MODBUS_Timout_Enable()
 */
static inline void MODBUS_Timout_Reset(void)
{
    MODBUS_Timout_Enable();
}

/**
 * @param none
 * @brief disables the timeout subsystem - re-enabled upon UART reception
 */
static inline void MODBUS_Timout_Disable(void)
{
    TCB2.CTRLA &= ~TCB_ENABLE_bm;
}

/**
 * @param timeout_us MODBUS/RTU timeout in µs - should be 3.5 character lengths
 * @brief initializes the timeout timer subsystem
 * @note uses TCB1 and TCB2 as well as Event channel 0 for cascading
 */
void MODBUS_Timeout_Init(uint32_t timeout_us)
{
    // TCB1 gives 50µs timer ticks for TCB2
    // TCB1 running at F_PER = F_CPU
    TCB1.CCMP = F_CPU / 20000UL - 1; // 50us prescaler
    TCB1.CTRLA = TCB_RUNSTDBY_bm | TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;
    TCB1.INTCTRL = 0;
    // TCB2 runs in steps of 50µs
    TCB2.CCMP = timeout_us / 50;
    TCB2.CTRLA = TCB_RUNSTDBY_bm | TCB_CASCADE_bm | TCB_CLKSEL_EVENT_gc;
    TCB2.CTRLB = TCB_CNTMODE_INT_gc;
    TCB2.INTCTRL = TCB_CAPT_bm;
    EVSYS.CHANNEL0 = EVSYS_CHANNEL0_TCB1_CAPT_gc;
    EVSYS.USERTCB2COUNT = EVSYS_CHANNEL00_bm;
}

/**
 * @brief interrupt service routine for MODBUS/RTU timeout
 * @note calls MODBUS_decode() for checking/decoding the received message
 */
ISR(TCB2_INT_vect)
{
    TCB2.INTFLAGS = 3;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        MODBUS_decode();
    }
    MODBUS_Timout_Disable();
}

/**
 * @brief interrupt service routine for UART reception
 * @note internal use only
 */
ISR(UART_INTVEC)
{
    uint8_t ch = UART.RXDATAL;
    MODBUS_Timout_Enable();

    mbBuffer[mbBufferPtr] = ch;
    if (mbBufferPtr < (mbBUFFSIZE - 1))
    {
        mbBufferPtr++;
    }
}

/**
 * @param baudrate - UART baudrate
 * @brief initializes the UART module
 * @note internal use only
 */
void MODBUS_UARTInit(uint32_t baudrate)
{
    UART_ROUTEREG = (UART_ROUTEREG & ~UART_PINROUTE_gm) | UART_PINROUTE_gc;
    UART_XDIRSET;
    UART_TXPINPULLUP;
    UART.BAUD  = UART_BAUD_CALC(baudrate);
    UART.CTRLB = USART_TXEN_bm | USART_RXEN_bm | USART_ODME_bm;
    UART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;
    UART.CTRLA = USART_RXCIE_bm | USART_RS485_bm | USART_LBME_bm;
}

/**
 * @param c - byte to send
 * @brief sends a single byte over the UART, blocking
 * @note internal use only
 */
void MODBUS_UART_BlockingSendByte(uint8_t c)
{
    while (!(UART.STATUS & USART_DREIF_bm));
    UART.TXDATAL = c;
}

/**
 * @param count - number of bytes from the buffer to send
 * @brief sends the contents of mbBuffer over the UART, blocking
 * @note internal use only
 */
int16_t MODBUS_UART_BlockingSendBuffer(uint16_t count)
{
    int16_t result = 0;
    uint8_t *ptr = mbBuffer;
    uint8_t ch;

    UART.CTRLB = USART_TXEN_bm | USART_ODME_bm;

    for (uint16_t i = 0; i < count; i++)
    {
        ch = mbBuffer[i];
        MODBUS_UART_BlockingSendByte(ch);
    }
    while (!(UART.STATUS & USART_TXCIF_bm));
    UART.STATUS = USART_TXCIF_bm;
    UART.CTRLB = USART_TXEN_bm | USART_RXEN_bm | USART_ODME_bm;
    return result;
}

/**
 * @param address Modbus/RTU address, 1..255
 * @return none
 * @brief initialize the Modbus/RTU server
 */
void MODBUS_init(uint8_t address)
{
    mbAdress = address;
    for (uint16_t i = 0; i < mbHOLDINGSIZE; i++)
    {
        mbHolding[i] = 4000+i; // debug
    }
    MODBUS_UARTInit(BAUD_RATE);
    MODBUS_Timeout_Init(UARTTIMEOUT);
    sei();
}

/**
 * @param none
 * @return none
 * @brief analyzes the received MODBUS package, prepares and sends a response
 * @note called from the timeout interrupt routine
 */
void MODBUS_decode(void)
{
    uint16_t crc;
    uint16_t dummy, start, count;
    if ((mbBufferPtr >= 4) && (mbBuffer[0] == mbAdress))
    {
        if (Modbus_CRC16(mbBuffer, mbBufferPtr) == 0)
        {
            // we have received a valid Modbus paket for our address
            switch (mbBuffer[1]) // function byte
            {
            case 1: // read coils
                mbBuffer[1] |= 0x80;
                mbBuffer[2] = 0x01; // illegal function
                crc = Modbus_CRC16(mbBuffer, 3);
                mbBuffer[3] = crc % 256;
                mbBuffer[4] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(5);
                break;
            case 2: // read discrete inputs
                mbBuffer[1] |= 0x80;
                mbBuffer[2] = 0x01; // illegal function
                crc = Modbus_CRC16(mbBuffer, 3);
                mbBuffer[3] = crc % 256;
                mbBuffer[4] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(5);
                break;
            case 3: // read holding registers
            case 4: // read input registers
                start = MODBUS16BIT(mbBuffer, 2);
                count = MODBUS16BIT(mbBuffer, 4);
                if (count > 127)
                {
                    count = 127;
                }
                if (((start + count) > mbHOLDINGSIZE) && (start < mbHOLDINGSIZE))
                {
                    count = mbHOLDINGSIZE - start;
                }
                mbBuffer[2] = 2 * count;
                for (uint16_t i = 0; i < count; i++)
                {
                    mbBuffer[2 * i + 3] = mbHolding[start + i] / 256;
                    mbBuffer[2 * i + 4] = mbHolding[start + i] % 256;
                }
                crc = Modbus_CRC16(mbBuffer, 2 * count + 3);
                mbBuffer[2 * count + 3] = crc % 256;
                mbBuffer[2 * count + 4] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(2 * count + 5);
                break;
            case 5: // write single coil
                mbBuffer[1] |= 0x80;
                mbBuffer[2] = 0x01; // illegal function
                crc = Modbus_CRC16(mbBuffer, 3);
                mbBuffer[3] = crc % 256;
                mbBuffer[4] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(5);
                break;
            case 6: // write single register
                start = MODBUS16BIT(mbBuffer, 2);
                mbHolding[start] = MODBUS16BIT(mbBuffer, 4);
                // echo message back
                MODBUS_UART_BlockingSendBuffer(mbBufferPtr);
                break;
            case 15: // write multiple coils
                mbBuffer[1] |= 0x80;
                mbBuffer[2] = 0x01; // illegal function
                crc = Modbus_CRC16(mbBuffer, 3);
                mbBuffer[3] = crc % 256;
                mbBuffer[4] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(5);
                break;
            case 16: // write multiple registers
                start = MODBUS16BIT(mbBuffer, 2);
                count = MODBUS16BIT(mbBuffer, 4);
                dummy = mbBuffer[6]; // byte count....?
                if (count > 127)
                {
                    count = 127;
                }
                if (((start + count) > mbHOLDINGSIZE) && (start < mbHOLDINGSIZE))
                {
                    count = mbHOLDINGSIZE - start;
                }
                // set registers
                for (uint16_t i = 0; i < count; i++)
                {
                    mbHolding[start + i] = MODBUS16BIT(mbBuffer, 2 * i + 7);
                }
                // prepare acknowledgement
                mbBuffer[4] = count / 256;
                mbBuffer[5] = count % 256;
                crc = Modbus_CRC16(mbBuffer, 6);
                mbBuffer[6] = crc % 256;
                mbBuffer[7] = crc / 256;
                MODBUS_UART_BlockingSendBuffer(8);
                break;
            default:
                break;
            }
        }
    }
    dummy = UART.RXDATAL; // empty receive buffer - just in case
    mbBufferPtr = 0;
}
