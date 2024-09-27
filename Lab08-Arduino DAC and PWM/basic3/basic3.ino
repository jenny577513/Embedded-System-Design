#include <Stepper.h>

const int buttonPinMoter = 3;
const int buttonPinRGB = 2;
const int ledPinR = 5;
const int ledPinG = 6;
const int redPin = 9, greenPin = 10, bluePin = 11;

volatile int buttonStateR = 0;
volatile int buttonStateRGB = 0;

// Button Pressed Duration
int timeR = 0;
int timeRGB = 0;

int readingR = 0;
int readingG = 0;

Stepper stepper(200, 7, 12, 8, 13);

void timerstepper(){

    if(buttonStateR) {
      timeR++;
    }
    else{
      if(timeR > 0) {
        timeR--;
      }
    }
}

void timerRGB(){
  
    if(buttonStateRGB) timeRGB++; 
    
    else{
      if(timeRGB > 0){
        timeRGB--;
        if(digitalRead(greenPin) == 0){ //off->on
          digitalWrite(redPin, 255);
          digitalWrite(greenPin, 255);
          digitalWrite(bluePin, 0);
        }
        else { //on->off
          digitalWrite(redPin, 0);
          digitalWrite(greenPin, 0);
          digitalWrite(bluePin, 0);
        }
      }
    }
}


void stepmoter(){
  if(buttonStateR)  stepper.step(200);
  else{
    if(timeR > 0) stepper.step(-200);
  }
}

void ledRGB(){
  
  if(buttonStateRGB) digitalWrite(redPin, HIGH);
  else{
    if(timeRGB <= 0){
      digitalWrite(redPin, 0);
      digitalWrite(greenPin, 0);
      digitalWrite(bluePin, 0);
    }
  }
}

ISR(TIMER1_COMPA_vect) {
  timerstepper();
  timerRGB();
}

void DebounceR(){
  Serial.print("Interrupt\n");
  long long last_int_time = 0;
  long long int_time = millis();

  if (int_time - last_int_time > 180 ) {  
    buttonStateR = !readingR;
  }

  last_int_time = int_time;
}

void DebounceRGB(){ // Debounce

  long long last_int_time = 0;
  long long int_time = millis();

  if (int_time - last_int_time > 180 ) {  
    buttonStateRGB = !readingG;
  }

  last_int_time = int_time;
}

void setup() {
  cli();
  Serial.begin(9600);
  pinMode(buttonPinMoter, INPUT);
  pinMode(buttonPinRGB, INPUT);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);


  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // 0.5 sec
  TIMSK1 |= (1<<OCIE1A); // enable timer compare
  
  attachInterrupt(digitalPinToInterrupt(buttonPinMoter), DebounceR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPinRGB), DebounceRGB, CHANGE);
  sei();
  stepper.setSpeed(100);
  //initial LED state
  digitalWrite(ledPinR, LOW);
  digitalWrite(ledPinG, LOW);

}

void loop(){
  readingR = digitalRead(buttonPinMoter);
  readingG = digitalRead(buttonPinRGB);
  stepmoter();
  ledRGB();
}
