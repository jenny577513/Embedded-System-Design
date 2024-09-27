#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6   //P1.0 & 1.6 to red LED
#define B1 BIT3     //P1.3 to button

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR &= ~ B1;    //Set P1.3 to input
    P1REN |= B1;   //Set P1.3 to use internal resistor
    P1OUT |= B1;  //Set P1.3 to use pull-up resistor

    P1DIR |= LED; //Set P1.0 & 1.6 for output
    P1OUT &= ~ LED; //Turn off LED (0)
    BCSCTL3 |= LFXT1S_2;
    BCSCTL2 |= SELM_3 + DIVM_3;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    //up mode, divide clk by 1, use ACLK, clear timer
    int state = 0;

    for (;;) { // Loop forever
        volatile unsigned int i,j;
        switch(state){

        case 0: //1st_red_on

           P1OUT ^= LED1;
           TA0CCR0 = 5999; //upper limit
           while (!(TA0CTL & TAIFG)) {} // Wait for time up
           TA0CTL &= ~TAIFG;  // Clear overflow flag
           state = 1;
           break;

        case 1: //1st_red_off
            P1OUT ^= LED1;
            TA0CCR0 = 5999; //upper limit
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            state = 2;
            break;

        case 2: //2st_red_on

           P1OUT ^= LED1;
           TA0CCR0 = 5999; //upper limit
           while (!(TA0CTL & TAIFG)) {} // Wait for time up
           TA0CTL &= ~TAIFG;  // Clear overflow flag
           state = 3;
           break;

        case 3: //2st_red_off
            P1OUT ^= LED1;
            TA0CCR0 = 5999; //upper limit
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            state = 4;
            break;

        case 4:

                TA0CCR0 = 11058; // 0.9sec
                P1OUT ^= LED2; // ON
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                P1OUT ^= LED2; // OFF
                TA0CCR0 = 5999;
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 0;
                break;

            }
    }


}

