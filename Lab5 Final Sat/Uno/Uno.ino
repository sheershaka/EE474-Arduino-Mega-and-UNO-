#include <stdio.h> 
#include <stdbool.h> 
//#include <TimerOne.h>
#include "optfft.c"

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
int currE = 0;

// Fields for Analog Reader
int tempPin;
int pPin;
int rPin;
int ePin;
int buttonPin;
int switchPin;
int but;
int preBut;
int sw;
int pulseCount;
int pulseRate;
int resCount;
int resRate;
int eCount;
int eRate;
bool high = false;
bool high2 = false;
bool high3 = false;
int ekgData[256] = {0};
int ekgImag[256] = {0};

float voltage;
float voltage2;
float voltage3;
int sample = 0;

// Fields for analog voltage output
int batteryPin;
int batteryPin2;

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
void measureBlood();
void readV();
void readR();
void readE();
int updatePulseRate();
int updateresRate();
int updateeRate();
void power();
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
  tempPin = A5; 
  batteryPin = 5;
  batteryPin2 = 10;
  pPin = 7;
  rPin = 6;
  ePin = A0;
  buttonPin = A1;
  switchPin = A4;
  pinMode(pPin,INPUT);
  pinMode(rPin,INPUT);
  //pinMode(batteryPin, OUTPUT);    
  pulseCount = 0; 
  voltage = 0; 
  pulseRate = 0;  
  // init, othereise start at 
  currTemp = 75;
  currSys = 80;
  currDia = 80;
  currPr = 100;
  
}

void loop() {  
  power();
  readV();
  readR();
  
  measureBlood();
  
  parseMessage();
  if (0 == strcmp(requestingTaskID, 'M')) {
    bool even = (unoCounter % 2 == 0);
    switch(requestedFunction) {
      case 'T':
        measureTemp(data);
        respondMessage("M","T",String(currTemp));
        break;
      case 'S':        
        respondMessage("M","S",String(currSys));               
        break;
      case 'D':        
        respondMessage("M","D",String(currDia));        
        break;
      case 'P':
        measurePr(data);
        respondMessage("M","P",String(currPr));
        break;
      case 'R':
        measureRr(data);
        respondMessage("M","R",String(currRr));
        break;    
//      case 'E':
//        readE();
//        for (int i = 0; i < 256; i++) {
//          respondMessage("M","E",String(ekgData[i]));
//          Serial.println(ekgData[i]);
//        }
//        break;   
    }    
  } else if (0 == strcmp(requestingTaskID, 'C')) {
    
  } else if (0 == strcmp(requestingTaskID, 'A')) {
    
  } else if (0 == strcmp(requestingTaskID, 'K')) {
    
  }

  //reset
  requestingTaskID = 0; 
  requestedFunction = 0; 
  //data = 0;
  even = false;
  
}



// measure function from mega
//update temp
void measureTemp(int currTempMega) {
  voltage = analogRead(tempPin) * (5/1023.0);
  currTemp = voltage * 12;
}

//update BloodPressure
void measureBlood() {
  sw = analogRead(switchPin) * (5/1023.0);
  but = analogRead(buttonPin) * (5/1023.0); 
  
//  Serial.println("sw:");
//  Serial.println(sw);
//  Serial.println("but:");
//  Serial.println(but);
  if ((preBut > 0) && (but == 0)) {
    if (sw > 0) { 
      currSys = currSys * 1.1;
      currDia = currDia * 1.1; 
    } else{      
      currSys = currSys * 0.9;   
      currDia = currDia * 0.9;             
    }
  }
  if (but == 0) {
    preBut = 0;
  } else preBut = 1;
 
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
  voltage = digitalRead(pPin); 
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
  voltage2 = digitalRead(rPin);
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
  resCount = 0; // init
  return resRate;
}

// power Output
void power(){
  analogWrite(batteryPin, 255);  
  analogWrite(batteryPin2, 255);
}

//read EKG
void readE() {
//  while (sample < 256) {
//    voltage3 = analogRead(ePin)/12;
//    respondMessage("M","E",String(voltage3-25));
//    //delayMicroseconds(100);    
//    delay(1);
//    sample++;
//  }
//  sample = 0;
//  int f = 400;
//  double fs = 2.5 * f;
//  for (int i = 0; i < 256; i++) {
//    voltage3 = 32*sin(2 * 3.14 * f * i/fs);    
//    respondMessage("M", "E", String(voltage3));
//  }
}
