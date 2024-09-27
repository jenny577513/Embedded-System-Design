#define KEY_ROWS 4
#define KEY_COLS 4

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void controlTask(void *pvParameters);
void displayTask(void *pvParameters);

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

char buf[2][16] = {0};
int button = 2;
int xAxis = A0, yAxis = A1;
int x = analogRead(xAxis);
int y = analogRead(yAxis);
int Press = digitalRead(button);


void dinoTask(void *pvParameters);
void displayTask(void *pvParameters);
void cactusTask(void *pvParameters);

int rowD, colD;
int rowCact, colCact;
int eggCol, eggRow;
int curRowCact = rowCact;
int curColCact = colCact;
int eggColStore[4] = {0};
int eggRowStore[4] = {0};
int i = 0;
int eggPro = 0;

// right: x > 1000
// left : x = 0
// up : y = 0
// down: y >1000

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  rowD = random(0, 1);
  colD = random(0, 15);
  rowCact = random(0, 1);
  colCact = random(2, 13);
  pinMode(button, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);
  lcd.setCursor(buf[colD][0], buf[0][rowD]); lcd.write(0);
  lcd.setCursor(buf[colCact][0], buf[0][rowCact]); lcd.write(1);

  xTaskCreate(dinoTask, "Dino", 128, NULL, 1, NULL);
  xTaskCreate(cactusTask, "Cactus", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "Display", 128, NULL, 1, NULL);

  vTaskStartScheduler();

}

void displayTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {

    lcd.clear();
    lcd.setCursor(buf[colD][0], buf[0][rowD]); lcd.write(0);
    lcd.setCursor(buf[colCact][0], buf[0][rowCact]); lcd.write(1);

    for (int i = 0; i < 1; i++) {
      for (int j = 0; j < 16; j++) {
        lcd.setCursor(buf[i][0], buf[0][j]); lcd.write(0);
      }
    }
                                      
  }
}

void dinoTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    x = analogRead(xAxis);
    y = analogRead(yAxis);
    Press = digitalRead(button);

    if (x == 0 || x > 1000 || y == 0 || y > 1000 || Press == 0) {
      if (Press == 0) {
        i++;
        if (i < 4) {
          eggPro = 1;
          eggCol = colD;
          eggRow = rowD;
//          if (i == 3) vTaskPrioritySet(xLCDHandle, 2);
        }
      }
      else {
        if (y == 0) rowD = 0;
        else if (y > 1000) rowD = 1;
        else if (x == 0) {
          if (colD > 0) colD--;
          else colD = 0;
        }
        else if (x > 1000) {
          if (colD < 15) colD++;
          else colD = 15;
        }
        buf[2][16] = {0};
        buf[colD][rowD] = 1;
      }
    }
  }
}

void cactusTask(void *pvParameters) {
  (void) pvParameters;

  int tempRow = rowD;
  int tempCol = colD;

  if (x == 0 || x > 1000 || y == 0 || y > 1000) {
    
    if ((rowD != tempRow) || (colD != tempCol)) {
      int tempLeft = curColCact - 1;
      int tempRight = curColCact + 1;
      int temp = colCact;
      colCact = random(tempLeft, tempRight);
      if (colCact == temp) {
        if (rowCact == 0) rowCact = 1;
        else rowCact = 0;
      }
    }
  }
}

void loop() {}
