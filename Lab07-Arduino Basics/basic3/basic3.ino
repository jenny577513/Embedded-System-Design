#define R 0
#define G 1
#define colorAmount 2

// constants won't change. They're used here to set pin numbers:
const int buttonPin[colorAmount] = {3, 2}; // the number of the pushbutton pin
const int ledPin[colorAmount] = {5, 6};    // the number of the LED pin

// Variables will change:
int buttonState[colorAmount] = {0};         // the current reading from the input pin
int lastButtonState[colorAmount] = {LOW, LOW}; // the previous reading from the input pin

int reading[colorAmount] = {0};

unsigned long lastDebounceTime[colorAmount] = {0};
unsigned long debounceDelay[colorAmount] = {200, 200};

// count press time
unsigned long startTime[colorAmount] = {0};
unsigned long endTime[colorAmount] = {0};
int blinkCount[colorAmount] = {0};
unsigned long blinkDuration[colorAmount] = {0};
int startDuration[colorAmount] = {0};

unsigned long currentTimeGlobal = 0;
int timerDuration = 100; //milliseconds
int blockTimer = false;

void setup()
{
  cli();

  Serial.begin(9600);
  for (int i = 0; i < colorAmount; i++)
  {
    pinMode(buttonPin[i], INPUT);
    pinMode(ledPin[i], OUTPUT);
    digitalWrite(ledPin[i], LOW); // set initial LED state
  }

  attachInterrupt(digitalPinToInterrupt(buttonPin[R]), handleClickR, CHANGE); // INT1 = pin 3
  attachInterrupt(digitalPinToInterrupt(buttonPin[G]), handleClickG, CHANGE); // INT0 = pin 2

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 1562;            // target for counting, 1562 = 100ms
  TCCR1B |= (1 << WGM12);        // turn on CTC
  TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);       // enable timer compare int.

  sei();
}

void loop(){}

void handleClickR()
{
  int color = R;

  blockTimer = true;

  unsigned long currentTime = millis();

  if ((currentTime - lastDebounceTime[color]) > debounceDelay[color])
  {
    reading[color] = digitalRead(buttonPin[color]);
    if (reading[color] != buttonState[color])
    {
      buttonState[color] = reading[color];

      if (buttonState[color] == HIGH)
      {
        startTime[color] = currentTime;
        digitalWrite(ledPin[color], HIGH);
        blinkCount[color] = 0;
      }
      else
      {
        endTime[color] = currentTime;
        digitalWrite(ledPin[color], LOW);
        float duration = ((float)(endTime[color] - startTime[color])) / 1000;
        blinkCount[color] = round(duration);
        if (color == R)
          Serial.print("Red : ");
        else if (color == G)
          Serial.print("Green : ");
        else
          Serial.print("Blue : ");
        Serial.print(duration);
        Serial.println(" sec");
      }
    }
  }
}

void handleClickG()
{
  int color = G;

  blockTimer = true;

  unsigned long currentTime = millis();

  if ((currentTime - lastDebounceTime[color]) > debounceDelay[color])
  {
    reading[color] = digitalRead(buttonPin[color]);
    if (reading[color] != buttonState[color])
    {
      buttonState[color] = reading[color];

      if (buttonState[color] == HIGH)
      {
        startTime[color] = currentTime;
        digitalWrite(ledPin[color], HIGH);
        blinkCount[color] = 0;
      }
      else
      {
        endTime[color] = currentTime;
        digitalWrite(ledPin[color], LOW);
        float duration = ((float)(endTime[color] - startTime[color])) / 1000;
        blinkCount[color] = round(duration);
        Serial.print("Green : ");
        Serial.print(duration);
        Serial.println(" sec");
      }
    }
}
}

void blink(int color){
  
  if ((blinkCount[color] > 0) && (buttonState[color] == LOW)){
    if (startDuration[color] == 0){
      blinkDuration[color] = currentTimeGlobal;
      startDuration[color] = 1;
    }
    else{
      unsigned long currentTime = currentTimeGlobal;
      if (currentTime - blinkDuration[color] >= 500){
          digitalWrite(ledPin[color], digitalRead(ledPin[color])^1);
          blinkDuration[color] = currentTime;
          blinkCount[color]--;
          startDuration[color] = 0;
      }
    }
  }
   //digitalWrite(ledPin[color],LOW);
}

ISR(TIMER1_COMPA_vect){
  currentTimeGlobal += timerDuration;
  //Serial.print(currentTimeGlobal);
  for (int i = 0; i < colorAmount; i++)
  {
    if ((blinkCount[i] > 0) && (buttonState[i] == LOW) && (blockTimer == false))
    {
      if (startDuration[i] == 0)
        blink(i);
      else
      {
        if (currentTimeGlobal - blinkDuration[i] >= 500)
          blink(i);
      }
    }
  }
}
