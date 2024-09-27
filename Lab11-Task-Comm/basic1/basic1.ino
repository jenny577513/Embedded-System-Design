#include <Arduino_FreeRTOS.h>
#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
#define KEY_ROWS 4 
#define KEY_COLS 4
LiquidCrystal_I2C lcd(0x3F,16,2);

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};

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
QueueHandle_t Global_Queue_Handle = 0; 
void controlTask(void *pvParameters);
void displayTask(void *pvParameters);

// Set I2C address and size
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.createChar(0, err_dragon);
  
  Global_Queue_Handle = xQueueCreate(5, sizeof(int));

  xTaskCreate(controlTask, "Control", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "Display", 128, NULL, 1, NULL);

}

void displayTask(void *pvParameters) {
  (void) pvParameters;
  int rowRec = 0;
  int colRec = 0;
  for(;;){
    if(xQueueReceive(Global_Queue_Handle,&rowRec,1000)) Serial.println(rowRec);
    if(xQueueReceive(Global_Queue_Handle,&colRec,1000)) Serial.println(colRec);
    lcd.clear();
    lcd.setCursor(colRec,rowRec); lcd.write(0);
    vTaskDelay(10);
  }
}

void controlTask(void *pvParameters) {
  (void) pvParameters;
  int rowD = 0;
  int colD = 0;

  for(;;){
    key = myKeypad.getKey();
    if(key) Serial.println(key);
    if(key == '2') rowD = 0;
    else if(key == '8') rowD = 1;
    else if(key == '4'){
      if(colD >0) colD--;
      else colD = 0;
    }
    else if(key == '6'){
      if(colD <15) colD++;
      else colD = 15;
    }
    xQueueSend(Global_Queue_Handle, &rowD, 1000);
    xQueueSend(Global_Queue_Handle, &colD, 1000);
}
}

void loop() {}
