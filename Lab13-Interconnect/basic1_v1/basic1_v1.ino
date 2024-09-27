#define portCHAR char  //https://forum.arduino.cc/index.php?topic=667888.0
#define KEY_ROWS 4
#define KEY_COLS 4

#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <queue.h>
#include <semphr.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void dinoTask(void *pvParameters);
void displayTask(void *pvParameters);
void cactusTask(void *pvParameters);

byte err_dragon[8] = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte cact[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

char buf[2][16] = {'\0'};
int xAxis = A0, yAxis = A1;
int button = 2;
int xVal = 0;
int yVal = 0;
int isPress = 0;

int rowDino = 0;
int colDino = 0;
int rowCact = 0;
int colCact = 0;
int colEgg = 0;
int rowEgg = 0;
int curRowCact = 0;
int curColCact = 0;
int storeRowEgg[4] = {'\0'};
int storeColEgg[4] = {'\0'};
int eggAmount = 0;
int eggProduct = 0;
int reset = 0;
int tempRow = 0;
int tempCol = 0;

SemaphoreHandle_t interruptSemaphore = 0;
SemaphoreHandle_t binary_sem = 0;

void setup() {
    Serial.begin(9600);
    pinMode(button, INPUT_PULLUP);  //return LOW when down

    randomSeed(analogRead(A3));
    rowDino = random(0, 1);
    colDino = random(0, 15);
    rowCact = random(0, 1);
    colCact = random(1, 13);
    while ((rowCact == rowDino) && (colCact == colDino)) {
        rowCact = random(0, 1);
        colCact = random(1, 13);
    }
    curRowCact = rowCact;
    curColCact = colCact;

    interruptSemaphore = xSemaphoreCreateMutex();
    vSemaphoreCreateBinary(binary_sem);

    buf[0][colDino] = colDino;
    buf[rowDino][0] = rowDino;
    buf[0][colCact] = colCact;
    buf[rowCact][0] = rowCact;

    lcd.init();
    lcd.backlight();
    lcd.createChar(0, err_dragon);
    lcd.createChar(1, cact);
    lcd.createChar(2, egg);
    lcd.createChar(3, broken);
    lcd.setCursor(buf[0][colDino], buf[rowDino][0]);
    lcd.write(0);
    lcd.setCursor(buf[0][colCact], buf[rowCact][0]);
    lcd.write(1);
    Serial.println(rowDino);
    Serial.println(colDino);

    xTaskCreate(dinoTask, (const portCHAR *)"Dino", 128, NULL, 1, NULL);
    xTaskCreate(cactusTask, (const portCHAR *)"Cactus", 128, NULL, 1, NULL);
    xTaskCreate(displayTask, (const portCHAR *)"Display", 128, NULL, 1, NULL);

    vTaskStartScheduler();
}

void dinoTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        xVal = analogRead(xAxis);
        yVal = analogRead(yAxis);
        isPress = digitalRead(button);

        tempRow = rowDino;
        tempCol = colDino;
        // Up / Down / Left / Right
        if ((xVal < 100 && yVal < 550) || (xVal > 1000 && yVal < 550) || (xVal < 550 && yVal > 1000) || (xVal > 500 && yVal < 100) || isPress == 0) {
            if (isPress == 0) {
                Serial.println("press");
                eggAmount = eggAmount + 1;

                storeRowEgg[eggAmount] = rowDino;
                storeColEgg[eggAmount] = colDino;
                if (eggAmount <= 3 && eggAmount >= 1) {
                    eggProduct = 1;
                    rowEgg = rowDino;
                    colEgg = colDino;

                    buf[0][storeColEgg[eggAmount]] = colEgg;
                    buf[storeRowEgg[eggAmount]][0] = rowEgg;

                    if ((storeColEgg[eggAmount] == colCact) && (storeRowEgg[eggAmount] == rowCact)) {
                        buf[2][16] = {'\0'};
                    } else if (eggAmount == 3) {
                        buf[2][16] = {'\0'};
                    }
                }
            } else {
                if (xVal < 100 && yVal < 550) {  // Up
                    if (rowDino == 1) {
                        rowDino = 0;
                    }
                } else if (xVal < 550 && yVal > 1000) {  // Left
                    if (colDino > 0) {
                        colDino--;
                    }
                } else if (xVal > 500 && yVal < 100) {  // Right
                    if (colDino < 15) {
                        colDino++;
                    }
                } else if (xVal > 1000 && yVal < 550) {  // Down
                    if (rowDino == 0) {
                        rowDino = 1;
                    }
                }
            }
            if (xSemaphoreTake(interruptSemaphore, 100)) {
                Serial.println("Dino got access");
                buf[2][16] = {'\0'};
                buf[rowDino][0] = rowDino;
                buf[0][colDino] = colDino;
                xSemaphoreGiveFromISR(binary_sem, NULL);
                xSemaphoreGive(interruptSemaphore);
            } else {
                Serial.println("Dino cannot");
            }
            vTaskDelay(30);
        }
    }
}

void cactusTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(binary_sem, 999999)) {
            if ((xVal < 100 && yVal < 550) || (xVal > 1000 && yVal < 550) || (xVal < 550 && yVal > 1000) || (xVal > 500 && yVal < 100) || isPress == 0) {
                if (isPress != 0) {
                    if ((rowDino != tempRow) || (colDino != tempCol)) {
                        int tempLeft = curColCact - 1;
                        int tempRight = curColCact + 1;
                        int temp = colCact;
                        colCact = random(tempLeft, tempRight);
                        if (colCact == temp) {
                            if (rowCact == 0)
                                rowCact = 1;
                            else
                                rowCact = 0;
                        }
                    }
                    buf[0][colCact] = colCact;
                    buf[rowCact][0] = rowCact;

                    int i = 0;
                    for (i = 1; i <= eggAmount; i++) {
                        if ((storeColEgg[i] == colCact) && (storeRowEgg[i] == rowCact)) {
                            buf[2][16] = {'\0'};
                        }
                    }
                }
            }
        }
    }
}

void displayTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(interruptSemaphore, 100)) {
            lcd.clear();
            Serial.println("Display got access");

            if (buf[rowDino][0] == buf[rowCact][0] && buf[0][colDino] == buf[0][colCact]) {
                lcd.setCursor(buf[0][colCact], buf[rowCact][0]);
                lcd.write(1);
            } else {
                int temp = 0;
                if (eggAmount > 0) {
                    for (temp = 1; temp <= eggAmount; temp++) {
                        if (buf[rowDino][0] != buf[storeRowEgg[temp]][0] || buf[0][colDino] != buf[0][storeColEgg[temp]]) {
                            lcd.setCursor(buf[0][colDino], buf[rowDino][0]);
                            lcd.write(0);
                        } else {
                            lcd.setCursor(buf[0][storeColEgg[temp]], buf[storeRowEgg[temp]][0]);
                            lcd.write(2);
                        }
                    }
                } else {
                    lcd.setCursor(buf[0][colDino], buf[rowDino][0]);
                    lcd.write(0);
                }

                lcd.setCursor(buf[0][colCact], buf[rowCact][0]);
                lcd.write(1);
            }

            if (eggProduct == 1) {
                int i = 0;
                for (i = 1; i <= eggAmount; i++) {
                    lcd.setCursor(buf[0][storeColEgg[i]], buf[storeRowEgg[i]][0]);
                    lcd.write(2);

                    if ((storeColEgg[i] == colCact) && (storeRowEgg[i] == rowCact)) {
                        lcd.setCursor(buf[0][storeColEgg[i]], buf[storeRowEgg[i]][0]);
                        lcd.write(3);

                        vTaskDelay(60);
                        // buf[2][16] = {'\0'};
                        lcd.clear();
                        lcd.setCursor(buf[2][16], buf[2][16]);
                        lcd.print(F("Game Over!"));

                        //reset
                        vTaskDelay(20);

                        rowDino = random(0, 1);
                        colDino = random(0, 15);
                        rowCact = random(0, 1);
                        colCact = random(1, 14);
                        while ((rowCact == rowDino) && (colCact == colDino)) {
                            rowCact = random(0, 1);
                            colCact = random(1, 14);
                        }
                        curRowCact = rowCact;
                        curColCact = colCact;
                        storeRowEgg[0] = storeRowEgg[1] = storeRowEgg[2] = storeRowEgg[3] = 0;
                        storeColEgg[0] = storeColEgg[1] = storeColEgg[2] = storeColEgg[3] = 0;
                        eggAmount = 0;
                        eggProduct = 0;

                        buf[0][colDino] = colDino;
                        buf[rowDino][0] = rowDino;
                        buf[0][colCact] = colCact;
                        buf[rowCact][0] = rowCact;

                        vTaskDelay(60);  // 1 sec
                    } else {
                        if (i == 3) {
                            vTaskDelay(30);
                            // buf[2][16] = {'\0'};
                            lcd.clear();
                            lcd.setCursor(buf[2][16], buf[2][16]);
                            lcd.print(F("Succeed!"));

                            //reset
                            vTaskDelay(20);  // 1 sec

                            rowDino = random(0, 1);
                            colDino = random(0, 15);
                            rowCact = random(0, 1);
                            colCact = random(1, 14);
                            while ((rowCact == rowDino) && (colCact == colDino)) {
                                rowCact = random(0, 1);
                                colCact = random(1, 14);
                            }
                            curRowCact = rowCact;
                            curColCact = colCact;
                            storeRowEgg[0] = storeRowEgg[1] = storeRowEgg[2] = storeRowEgg[3] = 0;
                            storeColEgg[0] = storeColEgg[1] = storeColEgg[2] = storeColEgg[3] = 0;
                            eggAmount = 0;
                            eggProduct = 0;

                            buf[0][colDino] = colDino;
                            buf[rowDino][0] = rowDino;
                            buf[0][colCact] = colCact;
                            buf[rowCact][0] = rowCact;

                            vTaskDelay(60);  // 1 sec
                        }
                    }
                }
            }

            xSemaphoreGive(interruptSemaphore);

        } else {
            Serial.println("Display cannot");
        }
        vTaskDelay(30);
    }
}

void loop() {}
