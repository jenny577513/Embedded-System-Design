const int buttonPinR = 3;
const int buttonPinG = 2;
const int ledPinR = 5;
const int ledPinG = 6;

volatile int buttonStateR = 0;
volatile int buttonStateG = 0;

// Button Pressed Duration
int timeR = 0;
int timeG = 0;

int readingR = 0;
int readingG = 0;

void timerR(){

    if(buttonStateR) {
      timeR++;
    }
    else{
      if(timeR > 0) {
        timeR--;
        digitalWrite(ledPinR, digitalRead(ledPinR)^1);
      }
    }
}

void timerG(){
  
    if(buttonStateG) timeG++; 
    else{
      if(timeG > 0){
        timeG--;
        digitalWrite(ledPinG, digitalRead(ledPinG)^1);
      }
    }
}

void ledR(){
  if(buttonStateR) digitalWrite(ledPinR, HIGH);
  else{
    if(timeR <= 0) digitalWrite(ledPinR, LOW);
  }
}

void ledG(){
  
  if(buttonStateG) digitalWrite(ledPinG, HIGH);
  else{
    Serial.print(timeG);
    if(timeG <= 0)digitalWrite(ledPinG, LOW);
  }
}

ISR(TIMER1_COMPA_vect) {
  timerR();
  timerG();
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

void DebounceG(){ // Debounce

  long long last_int_time = 0;
  long long int_time = millis();

  if (int_time - last_int_time > 180 ) {  
    buttonStateG = !readingG;
  }

  last_int_time = int_time;
}

void setup() {
  cli();
  Serial.begin(9600);
  pinMode(buttonPinR, INPUT);
  pinMode(buttonPinG, INPUT);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);


  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // 0.5 sec
  TIMSK1 |= (1<<OCIE1A); // enable timer compare
  
  attachInterrupt(digitalPinToInterrupt(buttonPinR), DebounceR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPinG), DebounceG, CHANGE);
  sei();
  
  //initial LED state
  digitalWrite(ledPinR, LOW);
  digitalWrite(ledPinG, LOW);

}

void loop(){
  readingR = digitalRead(buttonPinR);
  readingG = digitalRead(buttonPinG);
  ledR();
  ledG();
}
