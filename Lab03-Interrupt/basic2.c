#include <msp430.h>
#include <intrinsics.h> // Intrinsic functions

#define LED1 BIT0
#define LED2 BIT6
#define LED BIT0+BIT6   //P1.0 & 1.6 to red LED
#define B1 BIT3     //P1.3 to button

int state = 0;
int button = 0;
int count = 0;
int i=0;

int main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR &= ~ B1;    //Set P1.3 to input
    P1REN |= B1;   //Set P1.3 to use internal resistor
    P1OUT |= B1;  //Set P1.3 to use pull-up resistor

    P1DIR |= LED; //Set P1.0 & 1.6 for output
    P1OUT &= ~ LED; //Turn off LED (0)
    P1IE |= B1;              // P1.3 interrupt enabled
    P1IES |= B1;             // P1.3 Hi/lo edge
    P1IFG &= ~ B1;
    TA0CCTL0 = CCIE; // Enable interrupts(patten1)
    TA0CCR0 = 5999; // 0.5 sec (patten1)

    BCSCTL3 |= LFXT1S_2;
    BCSCTL2 |= SELM_3 + DIVM_3;
    TA0CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer0_A
    //up mode, divide clk by 1, use ACLK, clear timer

    TA1CTL = MC_1|ID_0|TASSEL_1|TACLR; //Setup Timer1_A
    //TA1CTL = MC_1|ID_0|TASSEL_2|TACLR; //Setup Timer1_A

    //BCSCTL1 = CALBC1_1MHZ;    // Set range
    //DCOCTL = CALDCO_1MHZ;
    _BIS_SR(GIE);
    while(1);

}
//按下按鈕，進到計時器1A開始計時，0A計時器還在跑(patten1)
#pragma vector = PORT1_VECTOR //button down
__interrupt void Port_1(void){

    //一開始按下不要有反應，1A開始計時
    if(button==0){
        TA1CCTL0 = CCIE; // Timer1A 開始計時(count)
        TA1CCR0 = 5999; //0.5 sec
        if(state==0) state = 0;
        //count = 0;
        P1IES &=~ B1; //反轉按鈕，放開會interrupt，開始閃燈
        button = 1;

    }
        //若按不到1秒，繼續執行patten1，並反轉按鈕
     else { //button ==1

         if(count>4){ //按超過3秒，放開再blink幾秒
             TA1CCTL0 &=~ CCIE;
             if(state!=6) state = 6;
             else if (state == 7) state = 7;
             P1IES |= B1;
             button = 0;
         }

         else if (count>0 && count<5){ //按不到3秒，狀態繼續patten1
             if((P1IN&&B1)!=0) count = 0;
             P1IES |= B1;
             button = 0;
        }
     }
       P1IFG &=~ B1;
}

// Interrupt service routine for CCR0 of Timer0_A3
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){ //button up + button down
    //patten1
    switch(state){

    case 0: //1st_red_on
        TA0CCR0 = 5999;
        P1OUT |= LED1;
        state = 1;
        break;

    case 1: //1st_red_off
        TA0CCR0 = 5999;
        P1OUT &=~ LED1; //OFF
        state = 2;
        break;

    case 2: //2st_red_on
        TA0CCR0 = 5999;
        P1OUT |= LED1;
        state = 3;
        break;

    case 3: //2st_red_off
        TA0CCR0 = 5999;
        P1OUT &=~ LED1; //OFF
        state = 4;
        break;

    case 4: //Green on
        P1OUT |= LED2; // ON
        TA0CCR0 = 10799;
        state = 5;
        break;

    case 5: //Green off
        TA0CCR0 = 5999;
        P1OUT &= ~ LED2;
        state = 0;
        break;

    case 6:

        if(i<count){
        TA0CCR0 = 5999;
        P1OUT |= LED; // on
        state = 7;
        i++;
        }
        else {
            state=0;
            count = 0;
            i=0;
            button = 0;
        }
        break;

    case 7:        //both off
        TA0CCR0 = 5999;
        P1OUT &=~ LED; // off
        if(i>=count){
            state = 0;
            count = 0;
            i=0;
            button = 0;
        }
        else {
            state = 6;
            i++;
        }
        break;

    default: break;

    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){

    count++;

}
