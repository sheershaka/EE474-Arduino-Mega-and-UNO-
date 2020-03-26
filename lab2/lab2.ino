// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.

// #include "tasks.h"
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library

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

//-------------------------
// for changing color in warning.
uint16_t tempColor = GREEN; 
uint16_t sysColor = GREEN;
uint16_t diaColor = GREEN;
uint16_t pulseColor = GREEN;
uint16_t batteryColor = GREEN;

// define Boolean type (from spec), notify all day
enum _myBool { FALSE = 0, TRUE = 1 };
typedef enum _myBool Bool;

// =======task struct=========
struct TaskStruct {
    void (*task)(void *);

    void *taskDataPtr;
};
typedef struct TaskStruct TCB;

//========sharedVar==========
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
// measure
unsigned int TemperatureRaw = 75;
unsigned int SystolicPressRaw = 80;
unsigned int DiastolicPressRaw = 80;
unsigned int PulseRateRaw = 50;
// display
double TemperatureCorrected = 0; // change it to double, not char* anymore, not NULL
double SystolicPressCorrected = 0;
double DiastolicPressCorrected = 0;
double PulseRateCorrected = 0;
// status
unsigned short BatteryState = 200;
// alarms
unsigned char BpOutOfRange = 0;
unsigned char TempOutOfRange = 0;
unsigned char PulseOutOfRange = 0;
// warning 
Bool BpHigh = FALSE;
Bool TempHigh = FALSE;
Bool PulseLow = FALSE;

//====tasks.h ========
void M(void *MData);
void C(void *CData);
void D(void *DData);
void W(void *WData);
void S(void *SData);
//==================




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

}

void loop() { 
  initlize();
}

void initlize() {
    TCB *taskQueue[6];
    
    // create instance for 5 tasks
    TCB MeasureTask;
    MeasureData measure;
    MeasureTask.taskDataPtr = (void *) &measure;
    MeasureTask.task = &M; // this is the placeholder for the fucntion M1
    measure.temperatureRaw = &TemperatureRaw;
    measure.systolicPressRaw = &SystolicPressRaw;
    measure.diastolicPressRaw = &DiastolicPressRaw;
    measure.pulseRateRaw = &PulseRateRaw;
    taskQueue[0] = &MeasureTask;
    
    TCB ComputeTask;
    ComputeData compute;
    ComputeTask.taskDataPtr = (void *) &compute;
    ComputeTask.task = &C; // placeholder
    //+++++
    compute.temperatureRaw = &TemperatureRaw;
    compute.systolicPressRaw = &SystolicPressRaw;
    compute.diastolicPressRaw = &DiastolicPressRaw;
    compute.pulseRateRaw = &PulseRateRaw;
    
    compute.temperatureCorrected = &TemperatureCorrected;
    compute.systolicPressCorrected = &SystolicPressCorrected;
    compute.diastolicPressCorrected = &DiastolicPressCorrected;
    compute.pulseRateCorrected = &PulseRateCorrected;
    
    taskQueue[1] = &ComputeTask;

    TCB DisplayTask;
    DisplayData displaydata;
    DisplayTask.taskDataPtr = (void *) &displaydata;
    DisplayTask.task = &D;
    displaydata.temperatureCorrected = &TemperatureCorrected;
    displaydata.systolicPressCorrected = &SystolicPressCorrected;
    displaydata.diastolicPressCorrected = &DiastolicPressCorrected;
    displaydata.pulseRateCorrected = &PulseRateCorrected;
    displaydata.batteryState = &BatteryState;
    taskQueue[2] = &DisplayTask;

    TCB WarningAlarmTask;
    WarningAlarmData warningdata;
    WarningAlarmTask.taskDataPtr = (void *)&warningdata;
    WarningAlarmTask.task = &W;
    warningdata.temperatureRaw = &TemperatureRaw;
    warningdata.systolicPressRaw = &SystolicPressRaw;
    warningdata.diastolicPressRaw = &DiastolicPressRaw;
    warningdata.pulseRateRaw = &PulseRateRaw;
    warningdata.batteryState = &BatteryState;
    taskQueue[3] = &WarningAlarmTask;

    TCB StatusTask;
    StatusData statusdata;
    StatusTask.taskDataPtr = (void *)&statusdata;
    StatusTask.task = &S;
    statusdata.batteryState = &BatteryState;
    taskQueue[4] = &StatusTask;
    
    // place holder, for extra task
    taskQueue[5] = 0x0; 

    schedule(taskQueue);
}

void schedule(TCB *taskQueue[6]) {
  unsigned int currentTask = 0;
  while (1) {
    while(currentTask < 6) {
      TCB *curr = taskQueue[currentTask];
      // execute each task except the one we use to place hold.
      if (curr!=0x0) {
        curr->task(curr->taskDataPtr); // call task function, takes data pointer for parameter.
      }
      currentTask++; // to next task
    }
    currentTask = 0; //reset
  }
  
}
//======= tasks.c==========
// millis() is used for System time
unsigned long wait  = 5000; // 5 sec delay set for one cycle

void M (void *MData) {
  static unsigned long minPassTime = 0;
  static unsigned int exeCounter = 0; // used for even/odd
  //
  int tempChange[2];
  int pulseChange[2];
  Bool systolicComplete = FALSE; 
  Bool diastolicComplete = FALSE;
  
  // execute for the first time OR every time pass the minPassTime, otherwise pass.
  if (minPassTime == 0 || millis() > minPassTime) {
    Serial.println("t1"); // for testing
    MeasureData *data = (MeasureData *)MData; //cast back

    //temp++++++
    if (*(data->temperatureRaw) > 50) {
      tempChange[0] = -2;
      tempChange[1] = 1;
    } else if (*(data->temperatureRaw) < 15) {
      tempChange[0] = 2;
      tempChange[1] = -1;
    }
    if (exeCounter % 2 == 0) {
      *(data->temperatureRaw) += tempChange[0];
    } else {
      *(data->temperatureRaw) += tempChange[1];
    }

    // systolic+++++++
    if (*(data->systolicPressRaw) > 100) {
      systolicComplete = TRUE;
      
    }
    if (!systolicComplete) {
      if (exeCounter % 2 == 0) {
        *(data->systolicPressRaw) += 3;
      } else {
        *(data->systolicPressRaw)--;
      }
    }

    // diastolic +++++
    if (*(data->diastolicPressRaw) < 40) {
      diastolicComplete = TRUE;
      if (systolicComplete) {
        systolicComplete = FALSE;
        systolicComplete = FALSE;
        *(data->diastolicPressRaw) = 80;
        *(data->systolicPressRaw) = 80;
        
      }
    }

    if (!diastolicComplete) {
      if (exeCounter % 2 == 0) {
        *(data->diastolicPressRaw) -= 2;
      } else {
        *(data->diastolicPressRaw)++;
      }
    }

    // Pulse Rate ++++++
    if (*(data->pulseRateRaw) > 40) {
      pulseChange[0] = 1;
      pulseChange[1] = -3;
    } else if (*(data->pulseRateRaw) < 15) {
      pulseChange[0] = -1;
      pulseChange[1] = 3;
    }
    if (exeCounter % 2 == 0) {
      *(data->pulseRateRaw) += pulseChange[0];
    } else {
      *(data->pulseRateRaw) += pulseChange[1];
    }
    Serial.println(*(data->pulseRateRaw));
 
    exeCounter++;
    minPassTime = millis() + wait;  
  }
}

void C(void *Cdata) {
  static unsigned long minPassTime = 0;
 
  if (minPassTime == 0 || millis() > minPassTime) {
    Serial.println("t2");
    ComputeData *data = (ComputeData *)Cdata; //cast back
    // conversion
    *(data->temperatureCorrected) =  5 + 0.75 * (*(data->temperatureRaw));
    *(data->systolicPressCorrected) = 9 + 2.0 * (*(data->systolicPressRaw));
    *(data->diastolicPressCorrected) = 6 + 1.5 * (*(data->diastolicPressRaw));
    *(data->pulseRateCorrected) = 8 + 3.0 * (*(data->pulseRateRaw));
    //
    Serial.println(*(data->temperatureCorrected));
    minPassTime = millis() + wait; 
  }
}

void D(void *DData) {
  static unsigned long minPassTime = 0;
 
  if (minPassTime == 0 || millis() > minPassTime) {
    Serial.println("t3");
    DisplayData *data = (DisplayData *)DData; //cast back
    // testing on monitor
    Serial.print("Temperature: ");
    Serial.println(*(data->temperatureCorrected));
    Serial.print("Systolic pressure: ");
    Serial.println(*(data->systolicPressCorrected));
    Serial.print("Diastolic pressure: ");
    Serial.println(*(data->diastolicPressCorrected));
    Serial.print("Pulse rate: ");
    Serial.println(*(data->pulseRateCorrected));
    Serial.print("Battery: ");
    Serial.println(*(data->batteryState));
    
    tft.fillScreen(BLACK);

    // cast data(double) to String();
    tft.setCursor(0,0);
    tft.setTextColor(tempColor);
    tft.println("Temperature: ");
    tft.print(String(*(data->temperatureCorrected)));
    tft.print(" C");
    
    tft.setCursor(0,50);
    tft.setTextColor(sysColor);
    tft.println("Systolic pressure: ");
    tft.print(String(*(data->systolicPressCorrected)));
    tft.print(" mmHg");
    
    tft.setCursor(0,100);
    tft.setTextColor(diaColor);
    tft.println("Diastolic pressure: ");
    tft.print(String(*(data->diastolicPressCorrected)));
    tft.print(" mmHg");
    
    tft.setCursor(0,150);
    tft.setTextColor(pulseColor);
    tft.println("Pulse Rate: ");
    tft.print(String(*(data->pulseRateCorrected)));
    tft.print(" BPM");

    tft.setCursor(0,200);
    tft.setTextColor(batteryColor);
    tft.println("Battery: ");
    tft.print(String(*(data->batteryState)));
    
    minPassTime = millis() + wait;
  }
}

void W(void *WData) {
  Serial.println("t4");
  WarningAlarmData *data = (WarningAlarmData *)WData; //cast back
  tft.setCursor(0,250);
  tft.print("State: ");
  tft.print("Normal");
  // range for ...
  //temp
  if (*(data->temperatureRaw) < 36.1 || *(data->temperatureRaw) > 37.8) {
    tempColor = RED;
  } else {
    tempColor = GREEN;
  }
  // sys
  if (*(data->systolicPressRaw) != 120) {
    sysColor = RED;
  } else {
    sysColor = GREEN;
  }

  //dia
  if (*(data->diastolicPressRaw) != 80) {
    diaColor = RED;
  } else {
    diaColor = GREEN;
  }

  // pulse
  if (*(data->pulseRateRaw) < 60 || *(data->pulseRateRaw) > 100) {
    pulseColor = RED;
  } else {
    pulseColor = GREEN;
  }

  // battery
  if (*(data->batteryState) < (200 * 0.2)) {
    batteryColor = RED;
  } else {
    batteryColor = GREEN;
  }

  
  delay(1000); // 4 more times during the intervel
}

void S(void *SData) {
  static unsigned long minPassTime = 0;
 
  if (minPassTime == 0 || millis() > minPassTime) {
    Serial.println("t5");
    StatusData *data = (StatusData *)SData; //cast back
    *(data->batteryState) -= 1;
    minPassTime = millis() + wait; 
  }
}
//===============
