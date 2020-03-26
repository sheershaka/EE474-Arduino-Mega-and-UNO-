#include <stdio.h> 
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
int currSys = 0; 
int currDia = 0; 
int currPr = 0;

// Fields for Analog Pulse Reader
int sensorPin;
int sensorValue;
int pulseCount;
int pulseRate;
bool high = false;
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
  sensorPin = A0; 
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
  readV();

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
        break;
      case 'D':
        measureDia(data);
        respondMessage("M","D",String(currDia));
        break;
      case 'P':
        measurePr(data);
        Serial.println("Pulse:");
        Serial.println(pulseCount);
        respondMessage("M","P",String(currPr));
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
  if (unoCounter % 5 == 0) {
    static int even = 0;
    
    if (currTempMega > 50) {
      tempChange[0] = -2;
      tempChange[1] = 1;
    } else if (currTempMega < 15) {
      tempChange[0] = 2;
      tempChange[1] = -1;
    }
    
    if (even % 2 == 0) {
      currTemp += tempChange[0];
    } else {
      currTemp += tempChange[1];
    }
  }
}
//update sys
void measureSys(int currSysMega) {
  if (unoCounter % 5 == 0) {
    // systolic+++++++
    if (currSysMega > 100) {
      systolicComplete = true;
      
    }
    if (!systolicComplete) {
      if (even % 2 == 0) {
        currSys += 3;
      } else {
        currSys--;
      }
    } else if (diastolicComplete) {
        systolicComplete = false;
        diastolicComplete = false;
        currDia = 80;
        currSys = 80;
    }
    
  }
}

// update dia
void measureDia(int currDiaMega) {
  if (unoCounter % 5 == 0) {
    // diastolic +++++
    if (currDiaMega < 40) {
      diastolicComplete = true;
      if (systolicComplete) {
        systolicComplete = false;
        diastolicComplete = false;
        currDia = 80;
        currSys = 80;
      }
    }

    if (!diastolicComplete) {
      if (even % 2 == 0) {
//        Serial.print("show1:");
//        Serial.println(*(data->temperatureRaw));
        currDia --;
        currDia --;
//        Serial.print("show2:");
//        Serial.println(*(data->temperatureRaw));
        
      } else {
        currDia++;
      }
    }
    
  }
}

// update pulse
void measurePr(int currPrMega) {
  if (unoCounter % 5 == 0) {
    currPr = updatePulseRate();
  }
}
//read function generator
void readV() {
  // convert to real voltage reading.
  voltage = analogRead(sensorPin) * (5/1023.0);
  Serial.println("Voltage:");
  Serial.println(voltage);  
  if (voltage*2 >= 3) {
    Serial.println("High:");
    Serial.println(high);
    Serial.println("Pulse:");
    Serial.println(pulseCount);
    if (!high) {
      pulseCount++;
      high = true;
    }
  }else high = false;
}
//
int updatePulseRate() {
  pulseRate = pulseCount;
  Serial.println("Pulse rate:");
  Serial.println(pulseRate);
  pulseCount = 0; // init
  return pulseRate;
}
