#include <Arduino_FreeRTOS.h>
//#include <Key.h>
//#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Set pins on I2C chip for LCD connections:
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};

const int PinL = A0;
const int PinR = A1;
int valL = 0; // photocell variable
int valR = 0; // photocell variable
int col1 = 8;
int col2 = 10;

TaskHandle_t xHandle;
void leftTask(void *pvParameters);
void rightTask(void *pvParameters);
void cactiTask(void *pvParameters);
void LCDTask(void *pvParameters);

LiquidCrystal_I2C lcd(0x3F,16,2);
// Set I2C address and size
void setup() { 
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, cactus);
  xTaskCreate(leftTask, "left", 128, NULL, 1, NULL);
  xTaskCreate(rightTask, "right", 64, NULL, 1, NULL);
  xTaskCreate(cactiTask, "cacti", 64, NULL, 1, &xHandle);
  xTaskCreate(LCDTask, "LCD", 64, NULL, 1, NULL);
}

void leftTask(void *pvParameters) {
  (void) pvParameters;
  pinMode(PinL, INPUT);
  
  for(;;){
    valL = analogRead(PinL);
    Serial.print(PinL);
  }
}
void rightTask(void *pvParameters) {
  (void) pvParameters;
  pinMode(PinR, INPUT);
  for(;;){
    valR = analogRead(PinR);
  }
}

void cactiTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){

    if(col1 >=0) col1--;
    else col1 = random(8, 15);

    if(col2 >=0) col2--;
    else col2 = random(8, 15);

    
    if(valL<600 && valR>600) vTaskDelay(30);
    else if(valL>600 && valR <600) vTaskDelay(10);
    else if(valL>600 && valR >600) vTaskDelay(20);
    else vTaskSuspend(xHandle);

  }
}

void LCDTask(void *pvParameters) {
  (void) pvParameters;
  for(;;){
    lcd.clear();
    if(valL >600 || valR>600) vTaskResume(xHandle);
    lcd.setCursor(col1,0); lcd.write(1);
    lcd.setCursor(col2,1); lcd.write(1);
    vTaskDelay(10);
  }
}

void loop() {}
