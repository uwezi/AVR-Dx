#include <avr/io.h>
#include <util/delay.h>
#include <nokia5110.h>

int main(void)
{
    char buffer[40];
    uint8_t vop = 0;

    /*
      display connected as
      Pin D0 - CLK (SCL)
      Pin D1 - DIN (SD)
      Pin D2 - D/C (DC)
      Pin D3 - CE  (SCE)
      Pin D4 - RST
    */
    NOKIA_init(&PORTD, 3, &PORTD, 4, &PORTD, 2, &PORTD, 1, &PORTD, 0, 0xc8, NOKIA_ORIENTATION_180);

    while (1)
    {
        vop++;
        sprintf(buffer, "VOP 0x%02x", vop);

        // uncomment to test different VOP settings
        // note the value for which the text is clearly readable
        //NOKIA_setVop(vop);
        NOKIA_print(0, 0, buffer, 0);  // print message in Nokia framebuffer
        NOKIA_update();
        _delay_ms(5);
   }

}