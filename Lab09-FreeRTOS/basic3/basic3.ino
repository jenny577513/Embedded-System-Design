#include <Arduino_FreeRTOS.h>
#define portCHAR char

void TaskA0(void *pvParameters);
void TaskA1(void *pvParameters);
void TaskSeven(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskRGB(void *pvParameters);

//Photocell & seg-7
const int PinL = A0; // 光敏電阻 A0
const int PinR = A1; // 光敏電阻 A1
int valL = 0; // photocell variable
int valR = 0; // photocell variable
int number = 0;
//ultrasonic sensor
const int trigPin = A2;
const int echoPin = A3;
long duration;
int distance;
//RGB
const int R = 9;
const int G = 10;
const int B = 11;
int mode = 1;

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
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  //initial display: 0
  for(int i = 0; i < 8; i++){
        digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
  }

  xTaskCreate(TaskSensor, (const portCHAR *) "task_Sensor", 128, NULL, 1, NULL);
  xTaskCreate(TaskA0, (const portCHAR *)"task_A0", 128, NULL, 1, NULL);
  xTaskCreate(TaskA1, (const portCHAR *)"task_A1", 128, NULL, 1, NULL);
  xTaskCreate(TaskSeven, (const portCHAR *)"task_Seven", 128, NULL, 1, NULL);
  xTaskCreate(TaskRGB, (const portCHAR *) "task_RGB", 128, NULL, 1, NULL);

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
  for(;;){
    if(valR<650 && valL>650 && distance < 15){
      if(number < 9) number++;
      else number = 9;
    }
    
    else if(valR>650 && valL<650 && distance < 15){
      if (number > 0) number--;
      else number = 0;
    }

    else if (distance > 15) number = 0;
    
    for(int i = 0; i < 8; i++){
      digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
    }
  
    vTaskDelay(10);
  
}
}

void TaskSensor(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
    /* Sets the trigPin on HIGH state for 10ms */
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    /* Reads Echo pin, returns sound travel time in ms */
    duration = pulseIn(echoPin, HIGH);
    /* Calculating the distance */
    distance = duration*0.034/2;
    Serial.print("Distance:");
    Serial.println(distance);
    vTaskDelay(1);

    if(distance <= 15){
      for(int i = 0; i < 8; i++){
        digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
      }  
    }
    else ;
  }
  
}

void TaskRGB(void *pvParameters) {
  (void) pvParameters;

  for(;;){
    if(number < 4){
      analogWrite(R,0);
      analogWrite(G,255);
      analogWrite(B,0);
    }
    else if (number >3 && number <7){
      analogWrite(R,255);
      analogWrite(G,255);
      analogWrite(B,0);
    }
    else if(number >7){
      analogWrite(R,255);
      analogWrite(G,0);
      analogWrite(B,0);
    }
  }

}

void loop() {}
