#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer                 // initialize LED off
    P1DIR |= 0x41;
    P1OUT &= ~0x41;
    BCSCTL3 |= LFXT1S_2;
    BCSCTL2 |= SELM_3 + DIVM_3;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    //up mode, divide clk by 1, use ACLK, clear timer

    for (;;) { // Loop forever
        volatile unsigned int i,j;
//Red *2
        i=0;
        while (i<4){
        P1OUT ^= LED1;
        TA0CCR0 = 5999; //upper limit
        while (!(TA0CTL & TAIFG)) {} // Wait for time up
        TA0CTL &= ~TAIFG;  // Clear overflow flag
        i++;
        }

        TA0CCR0 = 11058; // 0.9sec
        P1OUT ^= LED2; // ON
        while (!(TA0CTL & TAIFG)) {} // Wait for time up
        TA0CTL &= ~TAIFG;  // Clear overflow flag
        P1OUT ^= LED2; // OFF
        TA0CCR0 = 5999;
        while (!(TA0CTL & TAIFG)) {} // Wait for time up
        TA0CTL &= ~TAIFG;  // Clear overflow flag

    }

}
