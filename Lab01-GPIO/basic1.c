#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer                 // initialize LED off
    P1DIR |= 0x41;  // set P1.0 & 6 to outputs
    P1OUT = 0;

    volatile unsigned int i,j;

      for(;;) {
// Red LED
         j=0;
         while(j<4){ // on/off/on
         P1OUT ^= 0x01; // Toggle P1.0

         i = 20000;     // Delay
         do (i--);
         while (i != 0);
         j++;
         }
         P1OUT &= ~0x01; //P1.0 off

//Green LED
         P1OUT ^= 0x40; // Toggle P1.6

         i = 60000;     // Delay
         do (i--);
         while (i != 0);
         P1OUT &= ~0x41;
      }


}
