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
    IFG1 &= ~OFIFG;// Clear OSCFault flag
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    //up mode, divide clk by 1, use ACLK, clear timer

    TA1CTL = MC_1|ID_2|TASSEL_2|TACLR; //Setup Timer0_A
    //up mode, divide clk by 4, use SMCLK, clear timer
    BCSCTL1 = CALBC1_1MHZ;    // Set range
    DCOCTL = CALDCO_1MHZ;
    TA1CCR0 = 24000;


    int state = 0;
    int event = 11;
    //int i;
    int count = 0;

    for (;;) { // Loop forever

        //count =0;
        //button up
        if ((P1IN & B1) != 0){
            TA0CCR0 = 5999; //upper limit
            while (!(TA0CTL & TAIFG)) {} // Wait for time up
            TA0CTL &= ~TAIFG;  // Clear overflow flag
            TA1CTL &= ~TAIFG;
            event = 11;
            TA1R = 0;
        }

        //button down
        else if((P1IN & B1) == 0){

            TA1CCR0 = 24000;
               if((TA1CTL & TAIFG) && (count ==4)){
                   event = 12;
                   //P1OUT &=~ LED;
                   TA0CTL &= ~TAIFG;  // Clear overflow flag
                   TA1CTL &= ~TAIFG;
               }
               else{
                   TA0CCR0 = 5999;
                   while (!(TA0CTL & TAIFG)) {} // 0.5
                   TA0CTL &= ~TAIFG;
                   count ++;
                   if((P1IN & B1)!= 0) count =0;
               }
           }

        switch(state){

        case 0: //1st_red_on

            switch(event){
            case 11: //Timer0_A3_up
                P1OUT ^= LED1;
                state = 1;
                break;
            case 12: //button down
                state = 6;
                break;

            }
            break;

        case 1: //1st_red_off
            switch(event){
            case 11: //Timer0_A3_up
                P1OUT ^= LED1; //OFF
                state = 2;
            break;

            case 12: //button down
                state = 6;

                break;
            }
            break;

        case 2: //2st_red_on
            switch (event){
            case 11: //Timer0_A3_up
                P1OUT ^= LED1;
                 state = 3;
                break;
            case 12: //button down
                state = 6;
                break;
            }
            break;

        case 3: //2st_red_off
            switch (event){
            case 11: //Timer0_A3_up
                P1OUT ^= LED1; //OFF
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
                P1OUT |= LED2; // ON
                TA0CCR0 = 10799;
                if((P1IN & B1) == 0 && (TA1CTL & TAIFG)){
                    break;
                }
                while (!(TA0CTL & TAIFG)) {} // 0.9 sec
                TA0CTL &= ~TAIFG;  // Clear overflow flag
                state = 5;
                break;

            case 12: //button down
                state = 6;
                break;
            }

            break;

        case 5: //Green off
            switch(event){
            case 11:
                P1OUT &=~ LED2;
                state = 0;
                break;
            case 12:
                if(!(TA1CTL & TAIFG)) state = 6;
                break;
               }
            break;

        case 6: //both on & off

            while((P1IN & B1) == 0){//down
                P1OUT |= LED; // on
                TA0CCR0 = 5999;
                while (!(TA0CTL & TAIFG)) {} //0.5 sec
                TA0CTL &= ~TAIFG;

                P1OUT &=~ LED; // off
                while (!(TA0CTL & TAIFG)) {} // 0.5 sec
                TA0CTL &= ~TAIFG;
            }

                TA0CTL &= ~TAIFG;
                TA1CTL &= ~TAIFG;
                state = 0;
                event = 11;
                count =0;
                break;

        }

}
}
