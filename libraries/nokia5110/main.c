#include <avr/io.h>
#include <util/delay.h>
#include <nokia5110.h>
#include <avr/pgmspace.h>

void init(void)
{
  _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_4M_gc);

  NOKIA_init(
    &PORTD, 3, //sce_pin,
    &PORTD, 4, //rst_pin,
    &PORTD, 2, //dc_pin,
    &PORTD, 1, //sd_pin,
    &PORTD, 0, //scl_pin,
    0xd0,   //vop,
    NOKIA_ORIENTATION_180
  );
  NOKIA_print(0,40,"Hello World!",NOKIA_NORMAL);
  NOKIA_printtiny_P(0,34,PSTR("Hello World!"),NOKIA_NORMAL);
  NOKIA_update();
  _delay_ms(1000);
}

int main(void)
{
  init();
  uint8_t vop=0;
  char buffer[30];
  for (uint8_t i = 0; i < 24; i++)
  {
    NOKIA_puttinychar(i*4,0,'0'+i/10,NOKIA_NORMAL);
    NOKIA_puttinychar(i*4,6,'0'+i%10,NOKIA_NORMAL);
  }

  while (1)
  {
    NOKIA_setVop(vop);
    sprintf(buffer,"VOP=0x%02X",vop);
    NOKIA_print(0,20,buffer,NOKIA_NORMAL);
    NOKIA_printtiny(0,28,buffer,NOKIA_NORMAL);
    NOKIA_update();
    vop++;
    _delay_ms(20);
  }
}