#include <msp430.h> //Button up/down „³ LED on/off
#define LED BIT0+BIT6   //P1.0 & 1.6 to red LED
#define B1 BIT3     //P1.3 to button

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //Stop watchdog timer
    P1DIR &= ~ B1;    //Set P1.3 to input
    P1REN |= B1;   //Set P1.3 to use internal resistor
    P1OUT |= B1;  //Set P1.3 to use pull-up resistor

    P1DIR |= LED; //Set P1.0 & 1.6 for output
    P1OUT &= ~ LED; //Turn off LED (0)

    for (;;)
    {   //Loop forever

        volatile unsigned int i, j;

        //button down
        if ((P1IN & B1) == 0)
        {
            P1OUT ^= LED;
            i = 40000;     // Delay
            do
                (i--);
            while (i != 0);

        } // Yes

        //button up
        else
        {
            P1OUT &= ~ LED; //Turn off LED (0)
            j = 0;
            while (j < 4)
            {
                P1OUT ^= 0x01; // Toggle P1.0
                i = 20000;     // Delay
                do
                    (i--);
                while (i != 0 && ((P1IN & B1) != 0));
                j++;

            }

            P1OUT &= ~0x01; //P1.0 off
            P1OUT ^= 0x40; // Toggle P1.6

            i = 60000;     // Delay
            do
                (i--);
            while (i != 0 && ((P1IN & B1) != 0));
            P1OUT &= ~LED;

        }
    }
}

