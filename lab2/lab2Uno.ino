// Assignment 2 (UNO code)
// Sheershak, 1662387

#include <stdio.h>
#include <stdbool.h>


int unoCounter;

int currTemperature;
int currSystolic;
int currDiastolic;
int currPulseRate;

char ID;
char function;
int data;
bool even;

bool systolicComplete = false;
bool diastolicComplete = false;

int tempChange[2]; // first fall to 15 then reverse the values
int pulseChange[2]; // first fall to 15 then reverse the values

//functions
void measureTemp(int temperature);
void measureSystolicPressure(int systolicPressure);
void measureDiastolicPressure(int diastolicPressure);
void measurePulseRate(int pulseRate);

void measureTemp(int temperature) {
  if (unoCounter % 5 == 0) {
    if (temperature > 50){
      tempChange[0] = -2;
      tempChange[1] = 1;
    } else if (temperature < 15){
      tempChange[0] = 2;
      tempChange[1] = -1;
    }
    if (even) {
      currTemperature += tempChange[0];
    } else {
      currTemperature += tempChange[1];
    }
  }
}

void measureSystolicPressure(int systolicPressure) {
  if (unoCounter % 5 == 0) {
    if (systolicPressure > 100) {
      systolicComplete = true;
    } else {
      if (even) {
        currSystolic += 3;
      } else {
        currSystolic--;
      }
    }
  }
}

void measureDiastolicPressure(int diastolicPressure) {
  if (unoCounter % 5 == 0) {
    if (diastolicPressure < 40) { //reset
      diastolicComplete = true;
      if (systolicComplete) {
        systolicComplete = false;
        diastolicComplete = false;
        currSystolic = 80;
        currDiastolic = 80;
      }
    } else {
      if (even) {
        currDiastolic -= 2;
      } else {
        currDiastolic++;
      }
    }
  }
}

void measurePulseRate(int pulseRate) {
  if (unoCounter % 5 == 0) {
    if (pulseRate > 40){
      pulseChange[0] = 1;
      pulseChange[1] = -3;
    } else if (pulseRate < 15){
      pulseChange[0] = -1;
      pulseChange[1] = 3;
    }
    if (even) {
      currPulseRate += pulseChange[0];
    } else {
      currPulseRate += pulseChange[1];
    }
  }
}

void respond(String taskID, String funcToRun, String data) {
  Serial.println(taskID + ', ' + funcToRun + ', ' + data);
}

void decodeMessage() {
  while(Serial.available() > 0) {
    char cha = Serial.read();
    if (cha == 'U') {
      unoCounter++;
    } else if (cha = '<'){
      delay(20);
      ID = (char)Serial.read();
      Serial.read();
      delay(20);
      function = Serial.read();
      Serial.read();
      delay(20);
      data = Serial.parseInt();
      Serial.read();
    }
  }
}

void setup() { // Setup for Analog Pulse Reader
  Serial.begin(9600);
}

void loop() {
  decodeMessage();
  even = false; // make it true since 0 is even
  ID = 0;
  function = 0;
  data = 0;
  if (strcmp(ID, 'M') == 0) { //measure
    even = (unoCounter % 2 == 0);
    switch(function) {
      case 'T':
        measureTemp(data);
        respond('M', 'T', String(currTemperature));
        break;
      case 'S':
        measureSystolicPressure(data);
        respond('M', 'S', String(currSystolic));
        break;
      case 'D':
        measureDiastolicPressure(data);
        respond('M', 'D', String(currDiastolic));
        break;
      case 'P':
        measurePulseRate(data);
        respond('M', 'P', String(currPulseRate));
        break;
    }
  } else if (strcmp(ID, 'C') == 0) { //compute

  } else if (strcmp(ID, 'A') == 0) { //annuniciate
    
  } else if (strcmp(ID, 'B') == 0) { //status

  }
}
