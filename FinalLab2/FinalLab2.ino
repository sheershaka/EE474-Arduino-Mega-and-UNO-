// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.

/* One issue, it crash after running 152 seconds, 
 *  that is in the middle of battery goes from 48 tp 47. It stops
 *  and start printing "TFT LCD test, TFT size is 240x320", 
 *  and looping the setup() without delay.
*/

#include <stdio.h>
#include <stdbool.h> // for Boolean type
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library


int unoCounter = 0; // global counter

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

// =======Structs=========
struct TaskStruct {
    void (*task)(void *);

    void *taskDataPtr;
};
typedef struct TaskStruct TCB;

struct MeasureData {
  unsigned int *temperatureRaw;
  unsigned int *systolicPressRaw;
  unsigned int *diastolicPressRaw;
  unsigned int *pulseRateRaw;
};

struct ComputeData {
  unsigned int *temperatureRaw;
  unsigned int *systolicPressRaw;
  unsigned int *diastolicPressRaw;
  unsigned int *pulseRateRaw;
  double*temperatureCorrected;
  double*systolicPressCorrected;
  double*diastolicPressCorrected;
  double*pulseRateCorrected;
};

struct DisplayData {
  double*temperatureCorrected;
  double*systolicPressCorrected;
  double*diastolicPressCorrected;
  double*pulseRateCorrected;
  unsigned short *batteryState;
};

struct WarningAlarmData {
  unsigned int *temperatureRaw;
  unsigned int *systolicPressRaw;
  unsigned int *diastolicPressRaw;
  unsigned int *pulseRateRaw;
  unsigned short *batteryState;
};

struct StatusData {
  unsigned short *batteryState;
};

//=====initial value=====
// captilized intial to distinguish from mem of data struct

unsigned int TemperatureRaw;
unsigned int SystolicPressRaw;
unsigned int DiastolicPressRaw; //
unsigned int PulseRateRaw;

double TemperatureCorrected;
double SystolicPressCorrected;
double DiastolicPressCorrected;
double PulseRateCorrected;

unsigned short BatteryState;

unsigned char BpOutOfRange;
unsigned char TempOutOfRange;
unsigned char PulseOutOfRange;

bool BpHigh;
bool TempHigh;
bool PulseLow;
bool batteryOut;

//====tasks.h ========
void M(void *MData);
void C(void *CData);
void D(void *DData);
void W(void *WData);
void S(void *SData);
//==================
MeasureData measure;
ComputeData compute;
DisplayData displaydata;
WarningAlarmData warningdata;
StatusData statusdata;


// Delay for 100ms and update counter/time on peripheral system
void updateCounter(void) {
    delay(1000);
    unoCounter++;
    
}
//+++++++
TCB tasksArray[6];

void setup(void) {
    Serial.begin(9600); //Sets baud rate to 9600
    Serial.println(F("TFT LCD test")); //Prints to serial monitor

    //prints out tft size
    Serial.print("TFT size is ");
    Serial.print(tft.width());
    Serial.print("x");
    Serial.println(tft.height());

    tft.reset();
    tft.setTextSize(2);
    //prints out the current LCD driver version
    uint16_t identifier = tft.readID();
    if (identifier == 0x9325) {
        Serial.println(F("Found ILI9325 LCD driver"));
    } else if (identifier == 0x9328) {
        Serial.println(F("Found ILI9328 LCD driver"));
    } else if (identifier == 0x4535) {
        Serial.println(F("Found LGDP4535 LCD driver"));
    } else if (identifier == 0x7575) {
        Serial.println(F("Found HX8347G LCD driver"));
    } else if (identifier == 0x9341) {
        Serial.println(F("Found ILI9341 LCD driver"));
    } else if (identifier == 0x8357) {
        Serial.println(F("Found HX8357D LCD driver"));
    } else if (identifier == 0x0101) {
        identifier = 0x9341;
        Serial.println(F("Found 0x9341 LCD driver"));
    } else if (identifier == 0x1111) {
        identifier = 0x9328;
        Serial.println(F("Found 0x9328 LCD driver"));
    } else { //prints to serial monitor if wiring is bad or unknown LCD driver
        Serial.print(F("Unknown LCD driver chip: "));
        Serial.println(identifier, HEX);
        Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
        Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
        Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
        Serial.println(F("If using the breakout board, it should NOT be #defined!"));
        Serial.println(F("Also if using the breakout, double-check that all wiring"));
        Serial.println(F("matches the tutorial."));
        identifier = 0x9328;

    }
    tft.begin(identifier);
    tft.fillScreen(BLACK);

    // Initi
    TemperatureRaw = 75;
    SystolicPressRaw = 80;
    DiastolicPressRaw = 80;
    PulseRateRaw = 50;

    BatteryState = 200;

    BpHigh = false;
    TempHigh = false;
    PulseLow = false;
    batteryOut = false;

    
    //=====
    TCB MeasureTask;
    MeasureTask.taskDataPtr = (void *) &measure;
    MeasureTask.task = &M;
    measure.temperatureRaw = &TemperatureRaw;
    measure.systolicPressRaw = &SystolicPressRaw;
    measure.diastolicPressRaw = &DiastolicPressRaw; //
    measure.pulseRateRaw = &PulseRateRaw;

    TCB ComputeTask;
    ComputeTask.taskDataPtr = (void *) &compute;
    ComputeTask.task = &C;
    compute.temperatureRaw = &TemperatureRaw;
    compute.systolicPressRaw = &SystolicPressRaw;
    compute.diastolicPressRaw = &DiastolicPressRaw;
    compute.pulseRateRaw = &PulseRateRaw;
    compute.temperatureCorrected = &TemperatureCorrected;
    compute.systolicPressCorrected = &SystolicPressCorrected;
    compute.diastolicPressCorrected = &DiastolicPressCorrected;
    compute.pulseRateCorrected = &PulseRateCorrected; 
    
    TCB DisplayTask;
    DisplayTask.taskDataPtr = (void *) &displaydata;
    DisplayTask.task = &D;
    displaydata.temperatureCorrected = &TemperatureCorrected;
    displaydata.systolicPressCorrected = &SystolicPressCorrected;
    displaydata.diastolicPressCorrected = &DiastolicPressCorrected;
    displaydata.pulseRateCorrected = &PulseRateCorrected;
    displaydata.batteryState = &BatteryState;
    
    TCB WarningAlarmTask;
    WarningAlarmTask.taskDataPtr = (void *)&warningdata;
    WarningAlarmTask.task = &W;
    warningdata.temperatureRaw = &TemperatureRaw;
    warningdata.systolicPressRaw = &SystolicPressRaw;
    warningdata.diastolicPressRaw = &DiastolicPressRaw;
    warningdata.pulseRateRaw = &PulseRateRaw;
    warningdata.batteryState = &BatteryState;
    
    TCB StatusTask;
    StatusTask.taskDataPtr = (void *)&statusdata;
    StatusTask.task = &S;
    statusdata.batteryState = &BatteryState;

    /* SCHEDULE */
    tasksArray[0] = MeasureTask;
    tasksArray[1] = ComputeTask;
    tasksArray[2] = DisplayTask;
    tasksArray[3] = WarningAlarmTask;
    tasksArray[4] = StatusTask;
    // tasksArray[5] = NullTask;
}

void loop(void) { 
    for(int i = 0; i < 5; i++) { // QUEUE
        //unsigned long StartTime = millis();
        tasksArray[i].task(tasksArray[i].taskDataPtr);
        //unsigned long CurrentTime = millis();
        //unsigned long ElapsedTime = CurrentTime - StartTime;
        //Serial.print("Time taken for task " + (String)i + ": ");
        //Serial.println(ElapsedTime);
    }
    updateCounter();
    Serial.println(unoCounter);
}
//
//void loop() { 
//    TCB *taskQueue[6];
//    taskQueue[0] = &MeasureTask;
//    taskQueue[1] = &ComputeTask;
//    taskQueue[2] = &DisplayTask;
//    taskQueue[3] = &WarningAlarmTask;
//    taskQueue[4] = &StatusTask;
//    taskQueue[5] = 0x0;
//    schedule(taskQueue);
//}

//void schedule(TCB *taskQueue[6]) {
//  unsigned int currentTask = 0;
//  while (1) {
//    while(currentTask < 6) {
//      TCB *curr = taskQueue[currentTask];
//      // execute each task except the one we use to place hold.
//      if (curr!=0x0) {
//        curr->task(curr->taskDataPtr); // call task function, takes data pointer for parameter.
//      }
//      currentTask++; // to next task
//    }
//    currentTask = 0; //reset
//  }
//  
//}

//======= tasks.c==========
void M (void *MData) {
  static unsigned int tempChange[2];
  static unsigned int pulseChange[2];
  static bool systolicComplete = false; 
  static bool diastolicComplete = false;
  
  if (unoCounter % 5 == 0) {
    static int callCounter = 0;
    MeasureData *data = (MeasureData *)MData;

    unsigned int currtemperatureRaw = *(data->temperatureRaw);
    unsigned int currsystolicPressRaw = *(data->systolicPressRaw);
    unsigned int currdiastolicPressRaw = *(data->diastolicPressRaw);
    unsigned int currpulseRateRaw = *(data->pulseRateRaw);
  
    //temp++++
    if (currtemperatureRaw > 50) {
      tempChange[0] = -2;
      tempChange[1] = 1;
    } else if (currtemperatureRaw < 15) {
      tempChange[0] = 2;
      tempChange[1] = -1;
    }
    
    if (callCounter % 2 == 0) {
      currtemperatureRaw += tempChange[0];
    } else {
      currtemperatureRaw += tempChange[1];
    }

    // systolic+++++++
    if (currsystolicPressRaw > 100) {
      systolicComplete = true;
      
    }
    if (!systolicComplete) {
      if (callCounter % 2 == 0) {
        currsystolicPressRaw += 3;
      } else {
        currsystolicPressRaw--;
      }
    } else if (diastolicComplete) {
        systolicComplete = false;
        diastolicComplete = false;
        currdiastolicPressRaw = 80;
        currsystolicPressRaw = 80;
    }

    // diastolic +++++
    if (currdiastolicPressRaw < 40) {
      diastolicComplete = true;
      if (systolicComplete) {
        systolicComplete = false;
        diastolicComplete = false;
        currdiastolicPressRaw = 80;
        currsystolicPressRaw = 80;
      }
    }

    if (!diastolicComplete) {
      if (callCounter % 2 == 0) {
//        Serial.print("show1:");
//        Serial.println(*(data->temperatureRaw));
        currdiastolicPressRaw --;
        currdiastolicPressRaw --;
//        Serial.print("show2:");
//        Serial.println(*(data->temperatureRaw));
        
      } else {
        currdiastolicPressRaw++;
      }
    }

    // Pulse Rate
    if (currpulseRateRaw > 40) {
      pulseChange[0] = 1;
      pulseChange[1] = -3;
    } else if (currpulseRateRaw < 15) {
      pulseChange[0] = -1;
      pulseChange[1] = 3;
    }
    
    if (callCounter % 2 == 0) {
      currpulseRateRaw += pulseChange[0];
      
    } else {
      currpulseRateRaw += pulseChange[1];
    }

    *(data->temperatureRaw) = currtemperatureRaw;
    *(data->systolicPressRaw) = currsystolicPressRaw;
    *(data->diastolicPressRaw) = currdiastolicPressRaw;
    *(data->pulseRateRaw) = currpulseRateRaw;

    callCounter++; // update called times
    Serial.print("Overall Temperature: ");
    Serial.println(*(data->temperatureRaw));
    

  }
}

void C(void *Cdata) {
  if (unoCounter % 5 == 0) {
    ComputeData *data = (ComputeData *)Cdata;
    *(data->temperatureCorrected) =  5 + 0.75 * (*(data->temperatureRaw));
    *(data->systolicPressCorrected) = 9 + 2.0 * (*(data->systolicPressRaw));
    *(data->diastolicPressCorrected) = 6 + 1.5 * (*(data->diastolicPressRaw));
    *(data->pulseRateCorrected) = 8 + 3.0 * (*(data->pulseRateRaw));
  }
}

void D(void *DData) {
  if (unoCounter % 5 == 0) {
    DisplayData *data = (DisplayData *)DData;
    // testing on monitor
    Serial.println("Temperature: " + String(*(data->temperatureCorrected)));
    Serial.println("Systolic pressure: " + String(*(data->systolicPressCorrected)));
    Serial.println("Diastolic pressure: " + String(*(data->diastolicPressCorrected)));
    Serial.println("Pulse rate: " + String(*(data->pulseRateCorrected)));
    Serial.println("Battery: " + String(*(data->batteryState)));
    //
    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    tft.setCursor(0,0);
    TempHigh ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
    tft.println("Temperature: " + String(*(data->temperatureCorrected)) + " C");
    
    BpHigh ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
    tft.println("Systolic pressure: " + String(*(data->systolicPressCorrected)) + " mmHg");
    tft.println("Diastolic pressure: " +String(*(data->diastolicPressCorrected)) + " mmHg");
    
    PulseLow ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
    tft.println("Pulse Rate: " + String(*(data->pulseRateCorrected)) + " BPM");
    
    batteryOut ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
    tft.println("Battery: " + String(*(data->batteryState)));
  }
}

void W(void *WData) {
  WarningAlarmData *data = (WarningAlarmData *)WData;

  double temp = 5 + 0.75 * (*(data->temperatureRaw));
  double sys = 9 + 2.0 * (*(data->systolicPressRaw));
  double dias = 6 + 1.5 * (*(data->diastolicPressRaw));
  double pulse = 8 + 3.0 * (*(data->pulseRateRaw));

  //temp instead of double we use int here for simplicity
  if (temp < 36.1 || temp > 37.8) {
    TempHigh = true;
  } else {
    TempHigh = false;
  }
  
  // sys
  if (sys != 120 && dias != 80) {
    BpHigh = true;
  } else {
    BpHigh = false;
  }
  
  // pulse
  
  if (pulse < 60 || pulse > 100) {
    PulseLow = true;
  } else {
    PulseLow = false;
  }

  // battery
  if (*(data->batteryState) < 40) {
    batteryOut = true;
  } else {
    batteryOut = false;
  }

}

void S(void *SData) {
  if (unoCounter % 5 == 0) {
    StatusData *data = (StatusData *)SData;
    *(data->batteryState) -= 1;
  }
}
//=================
