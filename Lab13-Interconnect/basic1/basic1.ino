#include <advancedSerial.h>

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);

char input;
char test;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  //Arduino send to MSP430
  if(Serial.available()>0) {
      test = Serial.read();// read from user.
      mySerial.print(test);      
  }

  if(mySerial.available()>0) {
      input = mySerial.read(); //Send "HELLO\r\n"   
      Serial.print(input);
  }
  
  
}
