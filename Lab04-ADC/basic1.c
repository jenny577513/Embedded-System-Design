#include <msp430.h>

#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6
#define B1 BIT3

int state = 0;
int patten = 1;

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;    // Stop WDT

  P1DIR |= LED;
  P1OUT &= ~LED;

  ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
  ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
  ADC10AE0 |= 0x10;    // ADC10 option select
  ADC10CTL0 |= ENC; // Start sampling

//Timer1A(LED)
  TA1CCR0 = 5999;
  BCSCTL3 |= LFXT1S_2;
  TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
  TA1CCTL0 = CCIE; // Enable interrupts

//Timer0A
    TA0CCR0 = 2047;     // Sampling period
    TA0CCTL1 = OUTMOD_3;  // TA0CCR1 set/reset
    TA0CCR1 = 2046;       // TA0CCR1 OUT1 on time
    TA0CTL = MC_1|ID_0|TASSEL_2|TACLR; //SMCLK+DCO
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

  _BIS_SR(GIE);

for(;;){

  while(patten==1){

      switch(state){

              case 0: //1st_red_on
                  TA1CCR0 = 5999;
                  P1OUT |= LED1;
                  P1OUT &=~ LED2;
                  break;

              case 1: //1st_red_off
                  TA1CCR0 = 5999;
                  P1OUT &=~ LED;
                  break;

              case 2: //2st_red_on
                  TA1CCR0 = 5999;
                  P1OUT |= LED1;
                  P1OUT &=~ LED2;
                  break;

              case 3: //2st_red_off
                  TA1CCR0 = 5999;
                  P1OUT &=~ LED;
                  break;

              case 4: //Green on
                  TA1CCR0 = 10799;
                  P1OUT |= LED2; // ON
                  P1OUT &=~ LED1;
                  break;

              case 5: //Green off
                  TA1CCR0 = 5999;
                  P1OUT &=~ LED;
                  break;
      }
  }

   while(patten==2){

       switch(state){
          case 6:
              TA1CCR0 = 4199;
              P1OUT|= LED;
              break;

          case 7:
              TA1CCR0 = 7799;
              P1OUT &=~ LED;
              break;
       }
  }
}

}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    TA1CTL &= ~TAIFG;  // Clear overflow flag
    TA1R = 0;

    if(patten==1){
        if(state<6) state++;
        else state = 0;
    }
    else{ //patten == 2
        if(state==6)state=7;
        else state=6;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {

    if (ADC10MEM < 735){
        patten = 1;
    }

    else { //>740
        patten = 2;
  }
}
