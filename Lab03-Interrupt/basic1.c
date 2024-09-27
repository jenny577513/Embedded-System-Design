#include <msp430.h>
#include <intrinsics.h> // Intrinsic functions

#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6   //P1.0 & 1.6 to red LED
#define B1 BIT3     //P1.3 to button

int state = 0;
int button = 0;
int event = 11;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR &= ~ B1;    //Set P1.3 to input
    P1REN |= B1;   //Set P1.3 to use internal resistor
    P1OUT |= B1;  //Set P1.3 to use pull-up resistor

    P1DIR |= LED; //Set P1.0 & 1.6 for output
    P1OUT &= ~ LED; //Turn off LED (0)
    P1IE |= B1;              // P1.3 interrupt enabled
    P1IES |= B1;             // P1.3 Hi/lo edge
    P1IFG &= ~ B1;
    TA0CCR0 = 5999; //upper limit
    BCSCTL3 |= LFXT1S_2;
    BCSCTL2 |= SELM_3 + DIVM_3;
    TA0CCTL0 = CCIE; // Enable interrupts
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    //up mode, divide clk by 1, use ACLK, clear timer

    TA1CTL = MC_1|ID_2|TASSEL_2|TACLR; //Setup Timer0_A
    //up mode, divide clk by 4, use SMCLK, clear timer
    BCSCTL1 = CALBC1_1MHZ;    // Set range
    DCOCTL = CALDCO_1MHZ;
    TA1CCR0 = 12000; //1 sec

    _BIS_SR(GIE);

    while(1);
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){

    if(button == 0){
        P1IES &=~ B1;
        P1OUT |= LED;
        button =1;
        if(state ==7) state =7;
        else state = 6;
    }

    else if(button == 1){
        P1IES |= B1;
        P1OUT &=~ LED;
        button = 0;
        P1OUT |= LED1;
        state=0;
    }

    P1IFG &=~ B1;
}

// Interrupt service routine for CCR0 of Timer0_A3
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){

    switch(state){

    case 0: //1st_red_on
        TA0CCR0 = 5999;
        P1OUT |= LED1;
        state = 1;
        break;

    case 1: //1st_red_off

        P1OUT ^= LED1; //OFF
        state = 2;
        break;

    case 2: //2st_red_on

        P1OUT ^= LED1;
        state = 3;
        break;

    case 3: //2st_red_off

        P1OUT ^= LED1; //OFF
        state = 4;
        break;

    case 4: //Green on

        P1OUT |= LED2; // ON
        TA0CCR0 = 10799;
        if ((P1IN & B1) == 0 && (TA1CTL & TAIFG)) break;
        state = 5;
        break;

    case 5: //Green off
        TA0CCR0 = 5999;
        P1OUT &= ~ LED2;
        state = 0;
        break;

    case 6: //both on
        TA0CCR0 = 5999;
        P1OUT |= LED; // on
        state = 7;
        break;

    case 7: //both off
        P1OUT &=~ LED; // off
        state =6;
        break;

    }

}
