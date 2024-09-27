#include <msp430.h>

#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6
#define B1 BIT3

volatile unsigned int state = 0;
volatile unsigned int patten = 1;
volatile unsigned int button = 0; //button state
volatile unsigned int debug=0;
int adc[4]; //buffer
double c,t,avg;

void main(void) {
  WDTCTL = WDTPW + WDTHOLD;    // Stop WDT

  P1DIR |= LED;
  P1OUT &= ~LED;
  //button setting
  P1REN |= B1;
  P1OUT |= B1;
  P1IE |= B1;
  P1IES |= B1;
  P1IFG &= ~B1;

  ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
  ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
  ADC10AE0 |= 0x10;    // ADC10 option select
  ADC10CTL0 |= ENC;    // Start sampling
  ADC10DTC1 = 4;       // # of transfers
  ADC10SA = (int)adc;  // Buffer starting address


//Timer1A(LED)
  TA1CCR0 = 5999;
  BCSCTL3 |= LFXT1S_2;
  TA1CTL |= MC_1|ID_0|TASSEL_1|TACLR;
  TA1CCTL0 = CCIE; // Enable interrupts

//Timer0A
    TA0CCTL1 = OUTMOD_0;  // TA0CCR1 OUTPUT only
    TA0CTL = MC_3|ID_3|TASSEL_2|TACLR; //up-down mode
    TA0CCR0 = 42968; //5 sec
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 = DIVS_3;
    TA0CCTL0 = CCIE;
    debug = 1;
    TA0R = 0;

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
              TA1CCR0 = 4199; //0.35
              P1OUT|= LED;
              break;

          case 7:
              TA1CCR0 = 7799; //0.65
              P1OUT &=~ LED;
              break;
       }
  }
   while(patten == 3){
       TA1CCR0 = 2399; //0.2sec

       switch(state){

       case 8:
           P1OUT |= LED1;
           P1OUT &=~ LED2;

       case 9:
           P1OUT &=~ LED;
       }

   }
}

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    P1IFG &=~ B1;

    if(button == 0){ //按下
        TA0CTL &= ~TAIFG;
        P1IES &=~ B1; //反轉按鈕，放開會interrupt
        TA0R = 0;
        TA0CCTL0 = CCIE;
        debug = 1;
        TA0CCR0 = 42968;
        button = 1;
    }

    else if(button == 1){ //放開trigger
        TA0CTL &= ~TAIFG;
        TA0CCTL0 = 0;
        debug = 0;
        patten = 1;
        //state = 0;
        button = 0;
        P1IES |= B1;

    }

}
//Button interrupt >5秒進到Timer0A
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    //TA0R = 0;
    if(button == 1){ //按下>5秒
        TA0CCR1 = 1952; //每0.25秒
        TA0CCTL1 = OUTMOD_3; //trigger ADC10 測溫度
        TA0CCR0 = 1953;
        //TA0CCTL0 &=~ CCIE;
        //button = 2;
        //patten = 2;
        //state = 6;
        TA0CTL &= ~TAIFG;
    }

}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    TA1CTL &= ~TAIFG;  // Clear overflow flag
    TA1R = 0;

    if(patten == 2){ //patten == 2
        if(state==6)state=7;
        else state=6;
    }
    else if (patten == 3){ //patten 3
        if(state==8) state=9;
        else state=8;
    }
    else{ //patten == 1
        if(state<5) state++;
        else state = 0;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {

    t = ADC10MEM;
    c =((t*1.5/1023)-0.986)/0.00355;

    if(button == 1){
        avg = (adc[0] + adc[1] + adc[2] + adc[3]) / 4;
        ADC10SA = (int)adc;

        if(t<730){
            patten = 2;
            //state = 6;
        }
        else{// >740
            patten = 3;
            //state = 8;
        }
    }
    //ADC10CTL0 |= ENC + ADC10SC;
}

