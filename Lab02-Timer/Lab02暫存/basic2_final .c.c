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
    int event = 11;

    for (;;) { // Loop forever
        //button down

        if ((P1IN & B1) == 0) event = 12;

        switch(state){

        case 0: //1st_red_on

            switch(event){
            case 11: //Timer0_A3_up

                P1OUT ^= LED1;
                TA0CCR0 = 5999; //upper limit
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 1;
                break;
            case 12: //button down
                state = 6;
                break;
            }

        case 1: //1st_red_off
            switch(event){
            case 11: //Timer0_A3_up
            P1OUT ^= LED1; //OFF
            TA0CCR0 = 5999;
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            state = 2;
            break;
            case 12: ////button down
                state = 6;
                break;
            }
            break;

        case 2: //2st_red_on
            switch (event)
            {
            case 11: //Timer0_A3_up
                P1OUT ^= LED1;
                TA0CCR0 = 5999; //upper limit
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 3;
                break;
            case 12: //button down
                state = 6;
                break;
            }
            break;

        case 3: //2st_red_off
            switch(event){
            case 11: //Timer0_A3_up
            P1OUT ^= LED1; //OFF
            TA0CCR0 = 5999;
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            state = 4;
            break;
            case 12: ////button down
            state = 6;
            break;
            }
            break;

        case 4: //Green on
            switch(event){

            case 11: //button on
                P1OUT ^= LED2; // ON
                TA0CCR0 = 11058;
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 5;
                break;
            case 12: ////button down
                state = 6;
                break;
            }

            break;

        case 5: //Green off
            switch(event){
            case 11:
                P1OUT ^= LED2; // OFF
                TA0CCR0 = 5999;
                while (!(TA0CTL & TAIFG)) {}
                TA0CTL &= ~TAIFG;
                state = 0;
                break;
            case 12:
                state = 6;
                break;
               }
            break;

        case 6: //both on

                P1OUT |= LED;
                TA0CCR0 = 5999;
                while (!(TA0CTL & TAIFG)) {} // Wait for time up
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 7;
                break;

        case 7: //both off

            P1OUT &=~ LED;
            TA0CCR0 = 5999;
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            if((P1IN & B1) == 0) { //button down
                state = 6;
            }
            else {
                event = 11;
                state = 0;
            }
            break;
        }

}
}
