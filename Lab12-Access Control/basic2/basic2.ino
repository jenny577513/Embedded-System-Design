#define KEY_ROWS 4
#define KEY_COLS 4

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

TaskHandle_t xHandle;

void cactusTask(void *pvParameters);
void dinoTask(void *pvParameters);
void displayTask(void *pvParameters);

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

char buf[2][16] = {'\0'};
int xAxis = A0, yAxis = A1;
int x = analogRead(xAxis);
int y = analogRead(yAxis);
int Press = digitalRead(2); //button
int isPress = 0;

SemaphoreHandle_t gatekeeper = 0;
SemaphoreHandle_t binary_sem = 0;

void dinoTask(void *pvParameters);
void displayTask(void *pvParameters);
void cactusTask(void *pvParameters);
void eggMap();

int rowD, colD;
int rowCact, colCact;
int eggCol, eggRow;
int curRowCact = rowCact;
int curColCact = colCact;
int eggColStore[4] = {'\0'};
int eggRowStore[4] = {'\0'};
int i = 0;
int eggPro = 0;
int tempRow = 0;
int tempCol = 0;

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
  pinMode(9, OUTPUT); //buzzer
  pinMode(2, INPUT_PULLUP);
  curRowCact = rowCact;
  curColCact = colCact;

  gatekeeper = xSemaphoreCreateMutex();
  vSemaphoreCreateBinary(binary_sem);

  buf[0][colD] = colD;
  buf[rowD][0] = rowD;
  buf[0][colCact] = colCact;
  buf[rowCact][0] = rowCact;

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);
  lcd.setCursor(buf[0][colD], buf[rowD][0]); lcd.write(0);
  lcd.setCursor(buf[0][colCact], buf[rowCact][0]); lcd.write(1);

  attachInterrupt(digitalPinToInterrupt(2), eggMap, FALLING);

  xTaskCreate(dinoTask, "Control", 128, NULL, 1, &xHandle);
  xTaskCreate(cactusTask, "Cactus", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "Display", 128, NULL, 1, NULL);

  vTaskStartScheduler();

}

void displayTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    if (xSemaphoreTake(gatekeeper, 100)) {
      lcd.clear();

      if (buf[rowD][0] == buf[rowCact][0] && buf[0][colD] == buf[0][colCact]) {
        //overlap --> diaplay dino
        lcd.setCursor(buf[0][colCact], buf[rowCact][0]);
        lcd.write(1);
      }
      else {
        int temp = 0;
        if (i > 0) {
          for (temp = 1; temp <= i; temp++) {
            if (buf[rowD][0] != buf[eggColStore[temp]][0] || buf[0][colD] != buf[0][eggRowStore[temp]]) {
              lcd.setCursor(buf[0][colD], buf[rowD][0]);
              lcd.write(0);
            }
            else {
              lcd.setCursor(buf[0][eggColStore[temp]], buf[eggRowStore[temp]][0]);
              lcd.write(2);
            }
          }
        }
        else {
          lcd.setCursor(buf[0][colD], buf[rowD][0]);
          lcd.write(0);
        }

        lcd.setCursor(buf[0][colCact], buf[rowCact][0]);
        lcd.write(1);
      }

      if (eggPro == 1) { //'5' : display egg
        int j;
        for (j = 1; j <= i; j++) {
          lcd.setCursor(buf[0][eggColStore[j]], buf[eggRowStore[j]][0]); lcd.write(2);
          if (eggColStore[j] == colCact && eggRowStore[j] == rowCact) {
            lcd.setCursor(buf[0][eggColStore[j]], buf[eggRowStore[j]][0]); lcd.write(3);
            vTaskDelay(60);
            vTaskSuspend(xHandle);
            lcd.clear();
            lcd.setCursor(buf[2][16], buf[2][16]);
            lcd.print(F("Game Over"));
            tone(9, 500);
            vTaskDelay(60);  // 1 sec
            noTone(9);
            vTaskResume(xHandle);

            i = 0; eggPro = 0; //Restart
            lcd.clear();
            randomSeed(analogRead(A0));
            rowD = random(0, 2);
            colD = random(0, 15);
            eggRowStore[0] = eggRowStore[1] = eggRowStore[2] = eggRowStore[3] = 0;
            eggColStore[0] = eggColStore[1] = eggColStore[2] = eggColStore[3] = 0;
            buf[0][colD] = colD;
            buf[rowD][0] = rowD;
            buf[0][colCact] = colCact;
            buf[rowCact][0] = rowCact;
          }

          else if (j == 3) { // 3 eggs
            vTaskDelay(30);
            vTaskSuspend(xHandle);
            lcd.clear();
            lcd.setCursor(buf[2][16], buf[2][16]);
            lcd.print(F("Succeed!"));
            tone(9, 1000);
            vTaskDelay(60);  // 1 sec
            noTone(9);
            vTaskResume(xHandle);

            i = 0; eggPro = 0; //Restart
            lcd.clear();
            randomSeed(analogRead(A0));
            rowD = random(0, 2);
            colD = random(0, 15);
            eggRowStore[0] = eggRowStore[1] = eggRowStore[2] = eggRowStore[3] = 0;
            eggColStore[0] = eggColStore[1] = eggColStore[2] = eggColStore[3] = 0;
            buf[0][colD] = colD;
            buf[rowD][0] = rowD;
            buf[0][colCact] = colCact;
            buf[rowCact][0] = rowCact;
          }
        }
      }
      xSemaphoreGive(gatekeeper);
    }
    vTaskDelay(30);
  }
}


void dinoTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    x = analogRead(xAxis);
    y = analogRead(yAxis);
    //    Press = digitalRead(2);

    tempRow = rowD;
    tempCol = colD;

    if (x == 0 || x > 1000 || y == 0 || y > 1000 || isPress == 1) {
      Serial.print("Dino\n");
      if (isPress == 1) {
        //        i++;
        //        eggColStore[i] = colD;
        //        eggRowStore[i] = rowD;

        //        if (i < 4) {
        //          eggPro = 1;
        //          eggCol = colD;
        //          eggRow = rowD;
        //
        //          buf[0][eggColStore[i]] = eggCol;
        //          buf[eggRowStore[i]][0] = eggRow;

        if ((eggColStore[i] == colCact) && (eggRowStore[i] == rowCact)) {
          buf[2][16] = {'\0'};
        }
        else if (i == 3) buf[2][16] = {'\0'};
        isPress = 0;
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
      }

      if (xSemaphoreTake(gatekeeper, 100)) {
        buf[2][16] = {'\0'};
        buf[rowD][0] = rowD;
        buf[0][colD] = colD;
        xSemaphoreGive(binary_sem);
        xSemaphoreGive(gatekeeper);
      }
      vTaskDelay(30);
    }
  }
}


void cactusTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    if (x == 0 || x > 1000 || y == 0 || y > 1000 || Press == 0) {
      xSemaphoreTake(binary_sem, portMAX_DELAY);
      if (Press != 0) {
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
        buf[0][colCact] = colCact;
        buf[rowCact][0] = rowCact;

        int j = 0;
        for (j = 1; j <= i; j++) {
          if ((eggColStore[j] == colCact) && (eggRowStore[j] == rowCact)) {
            buf[2][16] = {'\0'};
          }
        }
      }
    }
  }
}
void Debounce() {
  Serial.print("Interrupt\n");
  long long last_int_time = 0;
  long long int_time = millis();

  if (int_time - last_int_time > 5000 ) {
    isPress = 1;
    i++;
  }
  else isPress = 0;

  last_int_time = int_time;
}

void eggMap() {
  Serial.print("ISR\n");
  Debounce();
  if (isPress == 1) {
    eggColStore[i] = colD;
    eggRowStore[i] = rowD;
    Serial.print(i);
    Serial.print("\n");
    if (i < 4) {
      eggPro = 1;
      eggCol = colD;
      eggRow = rowD;

      buf[0][eggColStore[i]] = eggCol;
      buf[eggRowStore[i]][0] = eggRow;
    }
  }
  else isPress = 0;
}

void loop() {}
