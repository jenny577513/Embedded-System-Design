
int PinL = A0; // 光敏電阻 A0
int PinR = A1; // 光敏電阻 A1
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
  {1,1,1,1,0,1,1} // 9
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
}

void Photocell(){

  if(valR<700 && valL>700){ // 若右邊光敏:暗，數字上升
        if(number < 9) number++;
        else number = 9;
        delay(100);
  }

  else if(valR>700 && valL<700){
        if (number > 0) number--;
        else number = 0;
        delay(100);
  }

}

void displayNum(){
 
 for(int i = 0; i < 8; i++){
        digitalWrite(pins[i],data[number][i] == 1 ? HIGH : LOW);
 }
  
}

void loop() {
  
  valL = analogRead(PinL);
  valR = analogRead(PinR);
  Serial.print("L:");
  Serial.println(valL);
  Serial.print("R:");
  Serial.println(valR);
  delay(100);

  //number = 0;
  Photocell();
  displayNum();

}
