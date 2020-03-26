/*#include <stdio.h> 
#include <stdbool.h> 
//#include <TimerOne.h>

#define STARTMESSAGE '>'
#define ENDTERM ','
#define ENDMESSAGE '<'

// GLOBAL COUNTER FOR UNO, Updated during interrupt service routines 
int unoCounter = 0;
char requestingTaskID = 0;
char requestedFunction = 0;
int data = 0;
bool even = false;

// data to send
int currTemp = 0; 
int currSys = 50; 
int currDia = 29; 
int currPr = 0;
int currRr = 0;

// Fields for Analog Pulse Reader
int tempPin;
int digPin;
int digPin2;
int buttonPin;
int switchPin;
int but;
int preBut;
int sw;
int pulseCount;
int pulseRate;
int resCount;
int resRate;
bool high = false;
bool high2 = false;

float voltage;
float voltage2;


//===== used in measure function
static unsigned int tempChange[2];
static unsigned int pulseChange[2];
static bool systolicComplete = false; 
static bool diastolicComplete = false;
//=======

// funtions
void measureTemp(int currTempMega) ; 
void measureSys(int currSysMega); 
void measureDia(int currDiaMega); 
void measurePr(int currPrMega); 
void measureRr(int currRrMega); 
void readV();
int updatePulseRate();
//===================================

// parse  message
void parseMessage() {
  while (Serial.available() > 0) {
    char currChar = Serial.read();
    if ('U' == currChar) {
      unoCounter++; //=================
    } else if ('>' == currChar) {
      delay(20);
      requestingTaskID = (char) Serial.read();
      Serial.read();

      delay(20);
      requestedFunction = Serial.read();

      delay(20);
      data = Serial.parseInt();
      Serial.read();
    }
  }
}

// respond message
void respondMessage ( String taskID , String funcToRun , String data ) { 
  //format
  Serial.println(STARTMESSAGE + taskID + ENDTERM + funcToRun + ENDTERM + data + ENDMESSAGE);
}

// ================================================================================

void setup() {
  Serial.begin(9600); //open  
  
  //analog reader, didn't check
  tempPin = A0; 
  digPin = 7;
  digPin2 = 6;
  buttonPin = 5;
  switchPin = 4;
  pinMode(digPin,INPUT);
  pinMode(digPin2,INPUT);
  pinMode(buttonPin,INPUT);
  pinMode(switchPin,INPUT);
  pulseCount = 0; 
  voltage = 0; 
  pulseRate = 0;  
  // init, othereise start at 
  currTemp = 75;
  currSys = 80;
  currDia = 80;
  currPr = 100;
  
}

void readV();
void readR();
  
void loop() {  
  readV();
  readR();
  
  parseMessage();
  if (0 == strcmp(requestingTaskID, 'M')) {
    bool even = (unoCounter % 2 == 0);
    switch(requestedFunction) {
      case 'T':
        measureTemp(data);
        respondMessage("M","T",String(currTemp));
        break;
      case 'S':
        measureSys(data);
        respondMessage("M","S",String(currSys));
        currSys = 50;
        break;
      case 'D':
        measureDia(data);
        respondMessage("M","D",String(currDia));
        currDia = 29;
        break;
      case 'P':
        measurePr(data);
//        Serial.println("Pulse:");
//        Serial.println(pulseCount);
//        Serial.println(currPr);
        respondMessage("M","P",String(currPr));
        break;
      case 'R':
        measureRr(data);
//        Serial.println("Pulse:");
//        Serial.println(pulseCount);
//        Serial.println(currRr);
        respondMessage("M","R",String(currRr));
        break;      
    }
  } else if (0 == strcmp(requestingTaskID, 'C')) {
    
  } else if (0 == strcmp(requestingTaskID, 'A')) {
    
  } else if (0 == strcmp(requestingTaskID, 'K')) {
    
  }

  //reset
  requestingTaskID = 0; 
  requestedFunction = 0; 
  data = 0;
  even = false;
}



// measure function from mega
//update temp
void measureTemp(int currTempMega) {
  voltage = analogRead(tempPin) * (5/1023.0);
  currTemp = voltage * 12;
}

//update sys
void measureSys(int currSysMega) {
  sw = digitalRead(switchPin);
  but = digitalRead(buttonPin);
  if ((preBut == 0) && (but == 1)) {
    if (sw == 1) { 
      currSys = currSys * 1.1;
    } else currSys = currSys * 0.9;        
  }
  preBut = but;
}

// update dia
void measureDia(int currDiaMega) {
  sw = digitalRead(switchPin);
  but = digitalRead(buttonPin);
  if ((preBut == 0) && (but == 1)) {
    if (sw == 1) { 
      currDia = currDia * 1.1;
    } else currDia = currDia * 0.9;        
  }
  preBut = but;
}

// update pulse
void measurePr(int currPrMega) {
  //if (unoCounter % 5 == 0) {
  currPr = updatePulseRate();
  //}
}
//read pulseRate
void readV() {
  // convert to real voltage reading.
  //voltage = analogRead(sensorPin) * (5/1023.0);
  voltage = digitalRead(digPin);
//  Serial.println("Voltage:");
//  Serial.println(voltage);
//  Serial.println("PR:");
//  Serial.println(pulseCount);  
  if (voltage > 0) {
    if (!high) {
      pulseCount++;
      high = true;
    }
  }else high = false;
}
//
int updatePulseRate() {
  pulseRate = pulseCount;
//  Serial.println("Pulse rate:");
//  Serial.println(pulseRate);
  pulseCount = 0; // init
  return pulseRate;
}

// update respiration
void measureRr(int currRrMega) {
  //if (unoCounter % 5 == 0) {
  currRr = updateresRate();
  //}
}
//read respration
void readR() {
  // convert to real voltage reading.
  //voltage = analogRead(sensorPin) * (5/1023.0);
  voltage2 = digitalRead(digPin2);
//  Serial.println("Voltage2:");
//  Serial.println(voltage2);
//  Serial.println("RR:");
//  Serial.println(resCount);  
  if (voltage2 > 0) {
    if (!high2) {
      resCount++;
      high2 = true;
    }
  }else high2 = false;
}
//
int updateresRate() {
  resRate = resCount;
//  Serial.println("Pulse rate:");
//  Serial.println(pulseRate);
  resCount = 0; // init
  return resRate;
}*/
