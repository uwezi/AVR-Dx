#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <nokia5110_hspi.h>

void plotduty(uint8_t x0, uint8_t y0, uint8_t duty)
{
  duty = duty/4;
  for (uint8_t i = 0; i < 10; i++)
  {
    NOKIA_setpixel(x0-1,y0-i);
    NOKIA_setpixel(x0+64,y0-i);
    NOKIA_setpixel(x0+duty,y0-i);
  }

  for (uint8_t i = 0; i < 64; i++)
  {
    if (i < duty)
    {
      NOKIA_setpixel(x0+i,y0-10);
    }
    else
    {
      NOKIA_setpixel(x0+i,y0);
    }
  }

}

int main(void)
{
  _delay_ms(1000);
  PORTA.DIRSET = PIN0_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN6_bm;
  uint16_t f=0;
  uint8_t vop=0;
  char textbuffer[40];

  PORTMUX.SPIROUTEA = PORTMUX_SPI0_DEFAULT_gc;


  NOKIA_init(
    &SPI0,
    &PORTA, 2, //rst_pin,
    &PORTA, 3, //dc_pin,
    0xc0, //vop,
   NOKIA_ORIENTATION_180

  );

  NOKIA_update();

  _delay_ms(1000);

  NOKIA_clear();

  NOKIA_print(0,40,"Hello World!",NOKIA_NORMAL);

  NOKIA_update();

  VREF.ADC0REF = VREF_REFSEL_VDD_gc;
  PORTD.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc ;
  PORTD.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc ;
  ADC0.CTRLA = (0 << ADC_CONVMODE_bp )
             | ADC_RESSEL_12BIT_gc
             | ADC_ENABLE_bm ;
  ADC0.CTRLB = 0;
  ADC0.CTRLC = ADC_PRESC_DIV20_gc;
  ADC0.CTRLD = 0;
  ADC0.CTRLE = 0;
  ADC0.SAMPCTRL = 0;
  ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
  ADC0.MUXNEG = ADC_MUXNEG_GND_gc;

  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm | TCA_SINGLE_CLKSEL_DIV1_gc;
  TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
  TCA0.SINGLE.PER = 255;
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;

  uint16_t adc0,adc1;
  while (1)
  {
    NOKIA_clearbuffer();

    ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0_COMMAND & ADC_STCONV_bm) {}
    adc0 = ADC0.RES / 16;

    ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0_COMMAND & ADC_STCONV_bm) {}
    adc1 = ADC0.RES / 16;

    sprintf(textbuffer,"ch1 %3d  ch2 %3d",adc0,adc1);
    NOKIA_printtiny(0,42,textbuffer,NOKIA_NORMAL);

    TCA0.SINGLE.CMP0 = adc0;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm | ((adc1/32)<<1);

    sprintf(textbuffer,"d/c =   %3d %%",(adc0*100)/255);
    NOKIA_print(0,12,textbuffer,NOKIA_NORMAL);

    plotduty(8,34,adc0);

    switch ((TCA0.SINGLE.CTRLA>>1) & 0b00000111)
    {
    case 0:
      f =  15625;
      break;
    case 1:
      f =   7812;
      break;
    case 2:
      f =   3906;
      break;
    case 3:
      f =   1953;
      break;
    case 4:
      f =    976;
      break;
    case 5:
      f =    244;
      break;
    case 6:
      f =     61;
      break;
    case 7:
      f =     15;
      break;

    default:
      f=0;
      break;
    }
    sprintf(textbuffer, "  f = %5d Hz",f);
    NOKIA_print(0,2,textbuffer,NOKIA_NORMAL);
    NOKIA_update();

  }

}
