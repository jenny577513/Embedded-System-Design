#include <Arduino_FreeRTOS.h>
#define portCHAR char

void TaskA0(void *pvParameters);
void TaskA1(void *pvParameters);
void TaskSeven(void *pvParameters);

const int PinL = A0; // 光敏電阻 A0
const int PinR = A1; // 光敏電阻 A1
int valL = 0; // photocell variable
int valR = 0; // photocell variable
int number = 0;

int pins[8] = {2, 3, 4, 5, 6, 7, 13, 12};//pins to 7-seg.
boolean data[10][8] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};


void setup(){
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(PinL, INPUT);
  pinMode(PinR, INPUT);

  //initial state
  for(int i = 0; i < 8; i++){
        digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
  }

  xTaskCreate(TaskA0, (const portCHAR *)"task_A0", 128, NULL, 1, NULL);
  xTaskCreate(TaskA1, (const portCHAR *)"task_A1", 128, NULL, 1, NULL);
  xTaskCreate(TaskSeven, (const portCHAR *)"task_Seven", 128, NULL, 1, NULL);

}

void TaskA0(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    valL = analogRead(PinL);
  }
}

void TaskA1(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    valR = analogRead(PinR);
  }
}

void TaskSeven(void *pvParameters) {
  (void) pvParameters;
  
for (;;) {

  if(valR<650 && valL>650){
        if(number < 9) number++;
        else number = 9;
  }

  else if(valR>650 && valL<650){
        if (number > 0) number--;
        else number = 0;
  }
  
  for(int i = 0; i < 8; i++){
     digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
  }
  
  vTaskDelay(50);
}
}

void loop() {}
