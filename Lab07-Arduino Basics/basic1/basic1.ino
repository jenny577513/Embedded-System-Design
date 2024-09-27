#define R 0
#define G 1
#define B 2
#define colorAmount 3

// constants won't change. They're used here to set pin numbers:
const int buttonPin[colorAmount] = {2, 3, 4}; // the number of the pushbutton pin
const int ledPin[colorAmount] = {5, 6, 7};    // the number of the LED pin

// Variables will change:
int buttonState[colorAmount] = {0};         // the current reading from the input pin
int lastButtonState[colorAmount] = {LOW, LOW, LOW}; // the previous reading from the input pin

int reading[colorAmount] = {0};

unsigned long lastDebounceTime[colorAmount] = {0};
unsigned long debounceDelay[colorAmount] = {200, 200, 200};

// count press time
unsigned long startTime[colorAmount] = {0};
unsigned long endTime[colorAmount] = {0};
int blinkCount[colorAmount] = {0};
unsigned long blinkDuration[colorAmount] = {0};
int startDuration[colorAmount] = {0};

unsigned long currentTimeGlobal = 0;
int timerDuration = 100; //milliseconds

void setup()
{
  Serial.begin(9600);
  pinMode(buttonPin[R], INPUT);
  pinMode(ledPin[R], OUTPUT);

  pinMode(buttonPin[G], INPUT);
  pinMode(ledPin[G], OUTPUT);

  pinMode(buttonPin[B], INPUT);
  pinMode(ledPin[B], OUTPUT);

  // set initial LED state
  digitalWrite(ledPin[R], LOW);
  digitalWrite(ledPin[G], LOW);
  digitalWrite(ledPin[B], LOW);

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 1562;            // target for counting (0.1 sec)
  TCCR1B |= (1 << WGM12);        // turn on CTC
  TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);       // enable timer compare int.
}

void loop()
{
  debounce(R);
  debounce(G);
  debounce(B);

  blink(R);
  blink(G);
  blink(B);
}

void debounce(int color)
{
  int currentTime = millis();

  reading[color] = digitalRead(buttonPin[color]);
  if (reading[color] != lastButtonState[color])
    lastDebounceTime[color] = currentTime;

  if ((currentTime - lastDebounceTime[color]) > debounceDelay[color])
  {
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
        blinkCount[color] = 2 * round(duration);
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
  
  lastButtonState[color] = reading[color];
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
}

ISR(TIMER1_COMPA_vect)
{
  currentTimeGlobal += timerDuration;
  //Serial.print(currentTimeGlobal);
}
