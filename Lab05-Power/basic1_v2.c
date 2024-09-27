#include <msp430.h>

#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6
#define B1 BIT3

volatile double c,t;
volatile unsigned int patten = 1;
volatile unsigned int state = 7;
double t0_st, t0_end, t3_st, t3_end;
double total0 = 0;
double total3 = 0;

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;

  P1DIR |= LED;
  P1OUT &= ~LED;

  ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
  ADC10CTL1 = SHS_0 + CONSEQ_0 + INCH_10; //single-channel-single-conversion
  ADC10AE0 |= 0x10;    // ADC10 option select
  ADC10CTL0 |= ENC; // Start sampling


//Timer1A(LED)
  TA1CCR0 = 5999; //0.5
  BCSCTL1 &= ~XTS;
  BCSCTL3 |= LFXT1S_2;
  TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
  TA1CCTL0 = CCIE; // Enable interrupts

//Timer0A(sensor)
    TA0CCR0 = 9599;     // 0.8
    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 9598;
    TA0CCTL0 = CCIE;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR;

    __enable_interrupt();

for(;;){

  while(patten==1){
            switch (state){

            case 0: //1st_red_on
                TA1CCR0 = 5999;
                P1OUT |= LED1;
                P1OUT &= ~ LED2;
                break;

            case 1: //1st_red_off
                TA1CCR0 = 5999;
                P1OUT &= ~ LED;
                break;

            case 2: //2st_red_on
                TA1CCR0 = 5999;
                P1OUT |= LED1;
                P1OUT &= ~ LED2;
                break;

            case 3: //2st_red_off
                TA1CCR0 = 5999;
                P1OUT &= ~ LED;
                break;

            case 4: //Green on
                TA1CCR0 = 10799;
                P1OUT |= LED2; // ON
                P1OUT &= ~ LED1;
                break;

            case 5: //Green off
                TA1CCR0 = 5999;
                P1OUT &= ~ LED;
                break;
            }
  }

   while(patten==2){
       switch(state){
          case 6:
              TA1CCR0 = 3599;  //0.3
              P1OUT|= LED;
              break;

          case 7:
              TA1CCR0 = 2399; //0.2
              P1OUT &=~ LED;
              break;
       }
  }
}

}
//every 0.4/0.8 sec sampling
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {

    TA0CTL &= ~TAIFG;
    //__bic_SR_register(LPM3_bits);  //out of LPM3
    ADC10CTL0 |= ENC + ADC10SC; // enable sampling
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    TA1CTL &= ~TAIFG;  // Clear overflow flag
    //TA1R = 0;

    //__bic_SR_register(LPM3_bits);  //out of LPM3

    if(patten==1){
        if(state<6) state++;
        else state = 0;
        t3_st = TA1R;
        __bic_SR_register_on_exit(LPM0_bits);  //out of LPM0

    }
    else{ //patten == 2
        if(state==6)state=7;
        else state=6;
        __bic_SR_register_on_exit(LPM3_bits);  //out of LPM3
        t0_st = TA1R;
        //t3_end = (TA1R - t3_st)/12000;
        //total3 = total3 + t3_end;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {

    t = ADC10MEM;
    c =((t*1.5/1023)-0.986)/0.00355;

    if (t < 730){
        patten = 1;
        TA0CCR0 = 9599; //0.8
        TA0CCR1 = 9598;

        t3_end = (TA1R - t3_st)/12000;
        total3 = total3 + t3_end;
        //__bic_SR_register_on_exit(LPM0_bits);  //out of LPM0
        //t3_st = TA1R;
        __bis_SR_register(LPM3_bits); //sleep in idle
        //__bis_SR_register(LPM3_bits); //enter LPM3
    }

    else {// C>28 emergency state
        patten = 2;
        TA0CCR0 = 4799; //0.4
        TA0CCR1 = 4798;
        
        t0_end = (TA1R - t0_st)/12000;
        total0 = total0 + t0_end;
        __bis_SR_register(LPM0_bits);
        //__bic_SR_register_on_exit(LPM3_bits);  //out of LPM3
        //t0_st = TA1R;

        //__bis_SR_register(LPM0_bits);  //enter LPM0

    }
}
