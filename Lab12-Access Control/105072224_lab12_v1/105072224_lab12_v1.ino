#define KEY_ROWS 4
#define KEY_COLS 4
#include <semphr.h>
#include <Arduino_FreeRTOS.h>
#include <Key.h>
#include <Keypad.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void cactusTask(void *pvParameters);
void dinoTask(void *pvParameters);
void displayTask(void *pvParameters);

byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};
char buf[2][16] = {'\0'};
int xAxis = A0, yAxis = A1;
int button = 2;
int Press = 0;

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

SemaphoreHandle_t interruptSemaphore = 0;
TaskHandle_t xHandle;
TaskHandle_t xLCDHandle;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  int rowD = random(0, 1);
  int colD = random(0, 15);
  int rowCact = random(0, 1);
  int colCact = random(2, 13);
  int curRowCact = rowCact;
  int curColCact = colCact;

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cact);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);
  lcd.setCursor(colD, rowD); lcd.write(0);
  lcd.setCursor(colCact, rowCact); lcd.write(1);

  xTaskCreate(controlTask, "Control", 128, NULL, 1, &xHandle);
  xTaskCreate(displayTask, "Display", 128, NULL, 1, &xLCDHandle);

  xQueueSend(Dinosour_Queue_Handle, &rowD, 1000);
  xQueueSend(Dinosour_Queue_Handle, &colD, 1000);
  xQueueSend(Cact_Queue_Handle, &rowCact, 1000);
  xQueueSend(Cact_Queue_Handle, &colCact, 1000);
  xQueueSend(CactCurrent_Queue_Handle, &curRowCact, 1000);
  xQueueSend(CactCurrent_Queue_Handle, &curColCact, 1000);
  vTaskStartScheduler();

}

void displayTask(void *pvParameters) {
  (void) pvParameters;
  int rowD, colD;
  int rowCact, colCact;
  int eggCol, eggRow;
  int eggColStore[4] = {0};
  int eggRowStore[4] = {0};
  int i = 0;
  int egg = 0;

  for (;;) {
    xQueueReceive(Dinosour_Queue_Handle, &rowD, 1000);
    xQueueReceive(Dinosour_Queue_Handle, &colD, 1000);
    xQueueReceive(Cact_Queue_Handle, &rowCact, 1000);
    xQueueReceive(Cact_Queue_Handle, &colCact, 1000);
    xQueueReceive(Egg_Queue_Handle, &i, 1000);
    xQueueReceive(Egg_Queue_Handle, &egg, 1000);
    xQueueReceive(Egg_Queue_Handle, &eggCol, 1000);
    xQueueReceive(Egg_Queue_Handle, &eggRow, 1000);

    lcd.clear();
    lcd.setCursor(colD, rowD); lcd.write(0);
    lcd.setCursor(colCact, rowCact); lcd.write(1);

    if (egg == 1) { //'5' : display egg
      eggColStore[i] = eggCol;
      eggRowStore[i] = eggRow;

      for (int j = 1; j <= i; j++) {
        lcd.setCursor(eggColStore[j], eggRowStore[j]); lcd.write(2);

        if (eggColStore[j] == colCact && eggRowStore[j] == rowCact) {
          lcd.setCursor(eggColStore[j], eggRowStore[j]); lcd.write(3);
          vTaskDelay(60);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Game Over"));
          vTaskPrioritySet(xLCDHandle, 1);
          vTaskDelay(60);  // 1 sec

          vTaskResume(xHandle);
        }
        else if (j == 3) { // 3 eggs
          vTaskDelay(30);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Succeed!"));
          vTaskPrioritySet(xLCDHandle, 1);
          vTaskDelay(60);  // 1 sec

          vTaskResume(xHandle);
        }
      }
    }
  }
}

void controlTask(void *pvParameters) {
  (void) pvParameters;
  int rowD, colD;
  int rowCact, colCact;
  int eggCol, eggRow;
  int curRowCact = 0;
  int curColCact = 0;
  int egg = 0;
  int i = 0;

  xQueueReceive(Dinosour_Queue_Handle, &rowD, 1000);
  xQueueReceive(Dinosour_Queue_Handle, &colD, 1000);
  xQueueReceive(Cact_Queue_Handle, &rowCact, 1000);
  xQueueReceive(Cact_Queue_Handle, &colCact, 1000);
  xQueueReceive(CactCurrent_Queue_Handle, &curRowCact, 1000);
  xQueueReceive(CactCurrent_Queue_Handle, &curColCact, 1000);

  for (;;) {
    int tempRow = rowD;
    int tempCol = colD;
    key = myKeypad.getKey();
    
    if (key == '2' || key == '8' || key == '5' || key == '4' || key == '6') {
      if (key == '5') {
        i++;
        if (i < 4) {
          egg = 1;
          eggCol = colD;
          eggRow = rowD;
          if (i == 3) vTaskPrioritySet(xLCDHandle, 2);
        }
      }
      else {
        
        if (key == '2') rowD = 0;
        else if (key == '8') rowD = 1;
        else if (key == '4') {
          if (colD > 0) colD--;
          else colD = 0;
        }
        else if (key == '6') {
          if (colD < 15) colD++;
          else colD = 15;
        }
        
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


      xQueueSend(Dinosour_Queue_Handle, &rowD, 1000);
      xQueueSend(Dinosour_Queue_Handle, &colD, 1000);

      xQueueSend(Cact_Queue_Handle, &rowCact, 1000);
      xQueueSend(Cact_Queue_Handle, &colCact, 1000);

      xQueueSend(Egg_Queue_Handle, &i, 1000);
      xQueueSend(Egg_Queue_Handle, &egg, 1000);
      xQueueSend(Egg_Queue_Handle, &eggCol, 1000);
      xQueueSend(Egg_Queue_Handle, &eggRow, 1000);

      if (i == 3 || rowCact == eggRow && colCact == eggCol) {
        vTaskSuspend(xHandle);

        i = 0; egg = 0; //Restart
        eggRow = 0; eggCol = 0;
        lcd.clear();
        randomSeed(analogRead(A0));
        rowD = random(0, 2);
        colD = random(0, 15);
        lcd.setCursor(colD, rowD); lcd.write(0);
        lcd.setCursor(colCact, rowCact); lcd.write(1);
      }
    }
  }
}

void loop() {}
