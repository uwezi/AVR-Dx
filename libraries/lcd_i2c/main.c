#include <avr/io.h>
#include <lcd_i2c.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <ds18b20.h>

// LCD debug buffer
char buffer[40];

void setup(void)
{
  _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FREQSEL_4M_gc);
  PORTA.DIRSET = PIN7_bm;

  //I2C_Init(&TWI0);
  LCD_init(16,2,&TWI0,0x27,0,1,2,4,5,6,7,3);
  LCD_setBacklight(1);
  LCD_clear();

  VREF.DAC0REF = VREF_ALWAYSON_bm | VREF_REFSEL_4V096_gc;
  _delay_us(50);
  PORTD.PIN6CTRL &= ~PORT_ISC_gm;
  PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
  DAC0.CTRLA = DAC_RUNSTDBY_bm | DAC_OUTEN_bm | DAC_ENABLE_bm;

  //======================================================
  VREF.ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL_4V096_gc;
  ADC0.CTRLA = ADC_RUNSTBY_bm | ADC_ENABLE_bm;
  ADC0.CTRLB = ADC_SAMPNUM_NONE_gc;
  ADC0.CTRLC = ADC_PRESC_DIV32_gc;
  ADC0.CTRLD = ADC_INITDLY_DLY256_gc | ADC_SAMPDLY_DLY0_gc;
  ADC0.CTRLE = ADC_WINCM_NONE_gc;
  ADC0.SAMPCTRL = 0;
  ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
  ADC0.MUXNEG = ADC_MUXNEG_GND_gc;
  ADC0.COMMAND = ADC_STCONV_bm;

  //============================================================
  DS18B20_init(&PORTA, 6);
  if (DS18B20_reset() == 0)
  {
    DS18B20_scanBus();
  }

      LCD_setCursor(0,0);
      sprintf(buffer, "%3d DS18B20 found", DS_devcount);
      LCD_print(buffer);
      LCD_setCursor(0,1);
      uint64_t addr = DS_addresses[0];
      for (int8_t i=60; i>=0; i-=4)
      {
        uint8_t dummy = (addr >> i) & 0x0f;
        LCD_print(utoa(dummy,buffer,16));
      }
      _delay_ms(1500);
}

int main(void)
{
    setup();
    uint16_t i=0;
    uint16_t dummy;
    int16_t temperature;
    LCD_clear();
    while (1)
    {
      dummy = i*4;

      // start conversion on all units
      DS18B20_reset();
      DS18B20_write(DS18B20_CMD_SKIPROM);

      DS18B20_write(DS18B20_CMD_CONVERTTEMP);

      LCD_setCursor(0,0);
      sprintf(buffer, "%4d Vs=%2ld.%03ld V", i, dummy/1000UL, dummy%1000UL);
      LCD_print(buffer);

      DAC0.DATA = i << 6;
      _delay_ms(1);
      ADC0_COMMAND |= ADC_STCONV_bm;
      while (ADC0_COMMAND & ADC_STCONV_bm);
      while(!DS18B20_readBit()) {_delay_us(1000);}
      DS18B20_reset();
      DS18B20_select(DS_addresses[0]); //first DS18B20 in list
      DS18B20_write(DS18B20_CMD_RSCRATCHPAD);
      //Read Scratchpad (only 2 first bytes)
      temperature  = DS18B20_read();
      temperature |= (DS18B20_read() << 8);

      dummy = ADC0.RES;
      LCD_setCursor(0,1);
      sprintf(buffer, "%3dC Vr=%2ld.%03ld V", temperature/16, dummy/1000UL, dummy%1000UL);
      LCD_print(buffer);

      i = (i+1) % 1024;
      _delay_ms(100);
    }

}