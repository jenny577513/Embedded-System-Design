#include <Arduino_FreeRTOS.h>
#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define KEY_ROWS 4 
#define KEY_COLS 4
#define LIGHT 600
LiquidCrystal_I2C lcd(0x3F,16,2);

// Set pins on I2C chip for LCD connections:
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte dinosaur[8]  = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};

const int PinL = A0;
const int PinR = A1;
int valL = 0; // photocell variable
int valR = 0; // photocell variable
int col1 = 9;
int col2 = 15;
int rowD = 0;
int GameOver = false;
int point = 0;

char key;
char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte colPins[KEY_COLS] = {9, 8, 7, 6};
byte rowPins[KEY_ROWS] = {13, 12, 11, 10}; 
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);
TaskHandle_t xHandle;
TaskHandle_t xLCDHandle;

void leftTask(void *pvParameters);
void rightTask(void *pvParameters);
void cactiTask(void *pvParameters);
void LCDTask(void *pvParameters);
void KeypadTask(void *pvParameters);

// Set I2C address and size
void setup() {
  Serial.begin(9600);
  pinMode(PinL, INPUT);
  pinMode(PinR, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, dinosaur);
  lcd.createChar(1, cactus);

  xTaskCreate(leftTask, "left", 64, NULL, 1, NULL);
  xTaskCreate(rightTask, "right", 64, NULL, 1, NULL);
  xTaskCreate(cactiTask, "cacti", 64, NULL, 1, &xHandle);
  xTaskCreate(LCDTask, "LCD", 96, NULL, 1, &xLCDHandle);
  xTaskCreate(KeypadTask, "keypad", 64, NULL, 1, NULL);

}

void leftTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    valL = analogRead(PinL);
  }
}
void rightTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    valR = analogRead(PinR);
  }
}

void cactiTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    
    if(col1 >0) col1--;
    else col1 = random(8, 15);

    if(col2 >0) col2--;
    else col2 = random(8, 15);

    if(valL<LIGHT && valR>LIGHT) vTaskDelay(30);
    else if(valL>LIGHT && valR <LIGHT) vTaskDelay(10);
    else if(valL>LIGHT && valR >LIGHT) vTaskDelay(20);
    else if(valL<LIGHT && valR <LIGHT) vTaskSuspend(xHandle);
    
    if(col1 == 0 && rowD == 0 || col2 == 0 && rowD == 1){
      GameOver = true;
      vTaskPrioritySet(xLCDHandle, 2);
      vTaskSuspend(xHandle);
    }
    else if(col1 == 0 && rowD == 1 || col2 == 1 && rowD == 0) point++;
  }
}

void LCDTask(void *pvParameters) {
  (void) pvParameters;
  
  for(;;){
    if(GameOver == false){
      lcd.clear();
      if(valL >LIGHT || valR>LIGHT) vTaskResume(xHandle);
      lcd.setCursor(col1,0); lcd.write(1);
      lcd.setCursor(col2,1); lcd.write(1);
      lcd.setCursor(0,rowD); lcd.write(0);
      vTaskDelay(10);
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);  lcd.print(F("Game Over"));
      lcd.setCursor(6,1);  lcd.print(F("Score:"));
      lcd.setCursor(13,1); lcd.print(point);
      vTaskDelay(120); // 2 sec
      
      point = 0;
      GameOver = false;
      col1 = 8; col2 = 10; rowD = 0;
      vTaskResume(xHandle);
      vTaskPrioritySet(xLCDHandle, 1);
    }
  }
}

void KeypadTask(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    key = myKeypad.getKey();
    if(key) Serial.println(key);
    if(key == '2') rowD = 0;
    else if(key == '8') rowD = 1;
  }
}

void loop() {}
