#include <stdio.h>
#include <stdbool.h> // for Boolean type
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <TimerThree.h> // hardware timer
// Uno will use <TimerOne.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define YP A3  // must be an analog pin
#define XM A2  // must be an analog pin
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define MINPRESSURE 10
#define MAXPRESSURE 1000

//Touch For New ILI9341 TP
#define TS_MINX 120
#define TS_MAXX 900

#define TS_MINY 70
#define TS_MAXY 920

#define STATUS_X 10
#define STATUS_Y 65

#define START_MESSAGE '>'
#define END_TERM ','
#define END_MESSAGE '<'

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD40

// Color definitions
#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

// Initialize screen, touchscreen and buttons
Elegoo_GFX_Button menuButtons[4];
//Elegoo_GFX_Button backButton[1];
Elegoo_GFX_Button dismissButton[1];
Elegoo_GFX_Button measureSelectButtons[5]; // Increased size by one to accommodate EKG (it was 3 before though)
Elegoo_GFX_Button AcknowledgeButton;

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int unoCounter = 0 ;

char requestingTaskID = 0;
char requestedFunction = 0;
int incomingData = 0;

const int sizeBuf = 8;

// for measure
unsigned int temperatureRawBuf[sizeBuf] = {75, 75, 75, 75, 75, 75, 75, 75};
unsigned int bloodPressRawBuf[2 * sizeBuf] = {80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80};
unsigned int pulseRateRawBuf[sizeBuf] = {50, 50, 50, 50, 50, 50, 50, 50};

int currTemp, currSys, currDias, currPR;

// flags using 0-5
unsigned short addFlags[6] = {1, 0, 1, 0, 1, 1};
unsigned short removeFlags[6] = {0};

// for display
unsigned char temperatureCorrectedBuf[sizeBuf];
unsigned char bloodPressCorrectedBuf[sizeBuf];
unsigned char pulseRateCorrectedBuf[sizeBuf];

// for Status
unsigned short batteryState = 200;

// for Alarm

bool tempOutOfRange = false;
bool bpOutOfRange = false;
bool pulseOutOfRange = false;

// for Warning
bool bpHigh = false;
bool tempHigh = false;
bool pulseLow = false;
bool battLow = false;

// for Keypad
unsigned short functionSelect = 0;
unsigned short measurementSelection = 0;
unsigned short alarmAcknowledge = 0;

/* INITIALIZATION*/
struct TaskStruct {
  void (*taskFuncPtr)(void*);
  void* taskDataPtr;
  struct TaskStruct* next;
  struct TaskStruct* prev;
}; typedef struct TaskStruct TCB;

struct MeasureStruct {
  unsigned int* temperatureRawPtr;
  unsigned int* bloodPressRawPtr;
  unsigned int* pulseRateRawPtr;
  unsigned short* measurementSelectionPtr;
}; typedef struct MeasureStruct MeasureTaskData;

struct ComputeStruct {
  unsigned int* temperatureRawPtr;
  unsigned int* bloodPressRawPtr;
  unsigned int* pulseRateRawPtr;
  unsigned char* temperatureCorrectedPtr;
  unsigned char* bloodPressCorrectedPtr;
  unsigned char* pulseRateCorrectedPtr;
  unsigned short* measurementSelectionPtr;
}; typedef struct ComputeStruct ComputeTaskData;

struct DisplayStruct {
  unsigned char* temperatureCorrectedPtr;
  unsigned char* bloodPressCorrectedPtr;
  unsigned char* pulseRateCorrectedPtr;
  unsigned short* batteryStatePtr;
}; typedef struct DisplayStruct DisplayTaskData;

struct WarningStruct {
  unsigned char* temperatureCorrectedPtr;
  unsigned char* bloodPressCorrectedPtr;
  unsigned char* pulseRateCorrectedPtr;
  unsigned short* batteryStatePtr;
}; typedef struct WarningStruct WarningTaskData;

struct StatusStruct {
  unsigned short* batteryStatePtr;
}; typedef struct StatusStruct StatusTaskData;

struct KeypadStruct {
  unsigned short* measurementSelectionPtr;
  unsigned short* alarmAcknowledgePtr;
}; typedef struct KeypadStruct KeypadTaskData;

struct CommStruct {
  unsigned char* temperatureCorrectedPtr;
  unsigned char* bloodPressCorrectedPtr;
  unsigned char* pulseRateCorrectedPtr;
  unsigned short* measurementSelectionPtr;
}; typedef struct CommStruct CommunicationTaskData;

// Head of TCB double linked list
TCB* linkedListHead = NULL;

// initialing TCB
TCB MeasureTask;
TCB ComputeTask;
TCB DisplayTask;
TCB AnnunciateTask;
TCB StatusTask;
TCB KeypadTask;
TCB NullTask;

MeasureTaskData measureData;
ComputeTaskData computeData;
DisplayTaskData displayData;
WarningTaskData warningData;
StatusTaskData statusData;
KeypadTaskData keypadData;
CommunicationTaskData commData;

void updateCounter(void) {
  Serial1.println("U");
  delay(1000);
  unoCounter++;
}

void requestMessage(String ID, String func, String data) {
  Serial1.println(START_MESSAGE + ID + END_TERM + func + END_TERM + data + END_MESSAGE);
  Serial1.println(START_MESSAGE + ID + END_TERM + func + END_TERM + data + END_MESSAGE);
}

void parseMessage() {
  while (0 == Serial1.available()) {
    // It freezes the system till the UNO responds
  }

  //  read incoming byte from the mega
  while (Serial1.available() > 0) {
    char currChar = Serial1.read();
    if ('>' == currChar) {
      delay(20);
      requestingTaskID = (char)Serial1.read();
      Serial1.read(); //Read over terminator
      // Serial.println(incomingData);
      delay(20);
      requestedFunction = Serial1.read();
      Serial1.read();
      // Serial.println(incomingData);
      delay(20);
      incomingData = Serial1.parseInt();
      Serial1.read();

      // Serial.println(incomingData);
    }
  }
}

void measureDataFunc(void* x) {
  MeasureTaskData* data = (MeasureTaskData*)x;
  MeasureTaskData temp = *data;

  unsigned int temperatureRawBufTemp[sizeBuf];
  unsigned int systolicPressRawBuf[sizeBuf];
  unsigned int diastolicPressRawBuf[sizeBuf];
  unsigned int pulseRateRawBufTemp[sizeBuf];

  // transfer to temporary variables
  for (int i = 0; i < sizeBuf * 2; i++) {
    if (i < sizeBuf) {
      temperatureRawBufTemp[i] = *(temp.temperatureRawPtr + i);
      systolicPressRawBuf[i] = *(temp.bloodPressRawPtr + i);
      pulseRateRawBufTemp[i] = *(temp.pulseRateRawPtr + i);
    } else {
      diastolicPressRawBuf[i - sizeBuf] = *(temp.bloodPressRawPtr + i);
    }
  }

  // Get most recent values --- SEND THESE TO UNO
  currTemp = temperatureRawBufTemp[0];
  currSys = systolicPressRawBuf[0];
  currDias = diastolicPressRawBuf[0];
  currPR = pulseRateRawBufTemp[0];

  // Tell the Uno to do all the work
  switch (measurementSelection) {
    case 0:
      requestMessage("M", "T", String(currTemp));
      parseMessage();
      currTemp = incomingData;
      delay(20);
      break;
    case 1:
      requestMessage("M", "S", String(currSys));
      parseMessage();
      currSys = incomingData;
      delay(20);
      requestMessage("M", "D", String(currDias));
      parseMessage();
      currDias = incomingData;
      break;
    case 2:
      requestMessage("M", "P", String(currPR));
      parseMessage();
      currPR = incomingData;
      delay(20);
      break;
    case 3:
      requestMessage("M", "T", String(currTemp));
      parseMessage();
      currTemp = incomingData;
      delay(20);
      requestMessage("M", "S", String(currSys));
      parseMessage();
      currSys = incomingData;
      delay(20);
      requestMessage("M", "D", String(currDias));
      parseMessage();
      currDias = incomingData;
      delay(20);
      requestMessage("M", "P", String(currPR));
      parseMessage();
      currPR = incomingData;
      delay(20);
      break;
  }

  // RESET message and add compute to addTask
  requestingTaskID = 0;
  requestedFunction = 0;
  incomingData = 0;

  // Shift older values back in the temporary buffer
  for (int i = sizeBuf - 1; i > 0; i--) {
    systolicPressRawBuf[i] = systolicPressRawBuf[i - 1];
    diastolicPressRawBuf[i] = diastolicPressRawBuf[i - 1];
    if (currTemp != temperatureRawBufTemp[0]) {
      temperatureRawBufTemp[i] = temperatureRawBufTemp[i - 1];
    }
    if (currPR != pulseRateRawBufTemp[0]) {
      pulseRateRawBufTemp[i] = pulseRateRawBufTemp[i - 1];
    }
  }

  // Update new values
  temperatureRawBufTemp[0] = currTemp;
  systolicPressRawBuf[0] = currSys;
  diastolicPressRawBuf[0] = currDias;
  pulseRateRawBufTemp[0] = currPR;

  // Update the data pointers
  for (int i = 0; i < sizeBuf * 2; i++) {
    if (i < sizeBuf) {
      *(temp.temperatureRawPtr + i) = temperatureRawBufTemp[i];
      *(temp.bloodPressRawPtr + i) = systolicPressRawBuf[i];
      *(temp.pulseRateRawPtr + i) = pulseRateRawBufTemp[i];
    } else {
      *(temp.bloodPressRawPtr + i) = diastolicPressRawBuf[i - sizeBuf];
    }
  }

  // Change Compute Flag to addTask when new data is measured
  addFlags[1] = 1;
}

void computeDataFunc(void* x) {
  ComputeTaskData* data = (ComputeTaskData*)x;
  ComputeTaskData temp = *data;
  int rawTemp = *(temp.temperatureRawPtr);
  int rawSys = *(temp.bloodPressRawPtr);
  int rawDia = *(temp.bloodPressRawPtr + 8) ;
  int rawPR = *(temp.pulseRateRawPtr) ;

  double correctedTemp = 5 + 0.75 * rawTemp ;
  *temp.temperatureCorrectedPtr = correctedTemp ;

  double correctedSys = 9 + 2.0 * rawSys ;
  *temp.bloodPressCorrectedPtr = correctedSys;

  double correctedDias = 6 + 1.5 * rawDia ;
  *(temp.bloodPressCorrectedPtr + 8) = correctedDias;

  double correctedPR = 8 + 3.0 * rawPR;
  *temp.pulseRateCorrectedPtr = correctedPR;
}

void displayDataFunc(void* x) {
  DisplayTaskData* data = (DisplayTaskData*)x;
  DisplayTaskData temp = *data;
  tft.setTextSize(1);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);

  // write code to create buttons
  menuButtons[0].initButton(&tft, 125, 55, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "MENU", 2);
  menuButtons[0].drawButton();

  menuButtons[1].initButton(&tft, 125, 125, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "ANNUNC", 2);
  menuButtons[1].drawButton();

  menuButtons[2].initButton(&tft, 125, 195, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "EM1", 2);
  menuButtons[2].drawButton();

  menuButtons[3].initButton(&tft, 125, 265, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "EM2", 2);
  menuButtons[3].drawButton();
}

void annunciateDataFunc(void* x) {
  //measurementSelection = 3; //to get all the values
  WarningTaskData* data = (WarningTaskData*)x;
  WarningTaskData temp = *data;

  if (tempHigh) {
    tft.setTextColor(RED);
  } else if (tempOutOfRange) {
    tft.setTextColor(ORANGE);
  } else {
    tft.setTextColor(GREEN);
  }
  tft.println("Temperature: " + (String)*temp.temperatureCorrectedPtr + " C");

  if (bpHigh) {
    tft.setTextColor(RED);
  } else if (bpOutOfRange) {
    tft.setTextColor(ORANGE);
  } else {
    tft.setTextColor(GREEN);
  }
  tft.println("Blood Pressure: " + (String)*temp.bloodPressCorrectedPtr + " mm Hg");

  if (pulseLow) {
    tft.setTextColor(RED);
  } else if (pulseOutOfRange) {
    tft.setTextColor(ORANGE);
  } else {
    tft.setTextColor(GREEN);
  }
  tft.println("Pulse Rate: " + (String)*temp.pulseRateCorrectedPtr + " BPM");

  if (battLow) {
    tft.setTextColor(RED);
  } else {
    tft.setTextColor(GREEN);
  }
  tft.println("Battery: " + (String)*temp.batteryStatePtr);

  double normalizedTemp = *temp.temperatureCorrectedPtr;
  if (normalizedTemp < 36.1 || normalizedTemp > 37.8) {
    tempOutOfRange = true;
    if (normalizedTemp > 40) {
      tempHigh = true;
    } else {
      tempHigh = false;
    }
  } else {
    tempOutOfRange = false;
    tempHigh = false;
  }

  int normalizedSystolic = *temp.bloodPressCorrectedPtr;
  if (normalizedSystolic > 130) {
    bpOutOfRange = true;
    if (normalizedSystolic > 160) {
      bpHigh = true;
    } else {
      bpHigh = false;
    }
  } else {
    bpOutOfRange = false;
    bpHigh = false;
  }

  double normalizedDiastolic = *(temp.bloodPressCorrectedPtr + 8);
  if (normalizedDiastolic > 90) {
    bpOutOfRange = true;
    if (normalizedDiastolic > 120) {
      bpHigh = true;
    } else {
      bpHigh = false;
    }
  } else {
    bpOutOfRange = false;
    bpHigh = false;
  }

  int normalizedPulse = *temp.pulseRateCorrectedPtr;
  if (normalizedPulse < 60 || normalizedPulse > 100) {
    pulseOutOfRange = true;
    if (normalizedPulse < 30) {
      pulseLow = true;
    } else {
      pulseLow = false;
    }
  } else {
    pulseOutOfRange = false;
    pulseLow = false;
  }

  if (batteryState < 40) {
    battLow = true;
  } else {
    battLow = false;
  }
}

void statusDataFunc(void* x) {
  StatusTaskData* data = (StatusTaskData*)x;
  StatusTaskData temp = *data;
  if (unoCounter % 5 == 0) {
    *temp.batteryStatePtr = 200 - (int)(unoCounter / 5);
  }
}

void menuView() {
  // Serial.println("Menu View started");
  measureSelectButtons[0].initButton(&tft, 125, 40, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Temperature", 2);
  measureSelectButtons[0].drawButton();

  measureSelectButtons[1].initButton(&tft, 125, 95, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Blood Pressure", 2);
  measureSelectButtons[1].drawButton();

  measureSelectButtons[2].initButton(&tft, 125, 150, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Pulse Rate", 2);
  measureSelectButtons[2].drawButton();

  bool staySensingPress = true;
  while (staySensingPress) {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      // scale from 0->1023 to tft.width
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
      // p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
      p.y = (map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
    }

    for (uint8_t b = 0; b < 3; b++) {
      if (measureSelectButtons[b].contains(p.x, p.y)) {
        // Serial.print("Pressing: "); Serial.println(b);
        measureSelectButtons[b].press(true); // tell the button it is pressed
        tft.setCursor(0, 0);
        tft.fillScreen(BLACK);
        addFlags[0] = 1;
        //addFlags[1] = 1;
        staySensingPress = false;
      } else {
        menuButtons[b].press(false);  // tell the button it is NOT pressed
      }
    }

    for (uint8_t b = 0; b < 3; b++) {
      if (measureSelectButtons[b].justPressed()) {
        if (0 == b) { // Temperature
          measurementSelection = 0;
          addFlags[3] = 1;
        } else if (1 == b) { // BP
          measurementSelection = 1;
          addFlags[3] = 1;
        } else if (2 == b) { // Pulse
          measurementSelection = 2;
          addFlags[3] = 1;
        }
        measureSelectButtons[b].press(false);
      }
    }
  }
}

void keypadDataFunc(void* x) {
  bool pressed = false;
  KeypadTaskData* data = (KeypadTaskData*)x;
  KeypadTaskData temp = *data;
  while (!pressed) {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
      p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);//(tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
    }
    for (uint8_t b = 0 ; b < 4; b++) {
      if (menuButtons [b].contains (p.x, p.y)) {
        menuButtons [b].press(true);
        pressed = true ;
      } else {
        menuButtons[b].press(false);
      }
    }
    for (uint8_t b = 0; b < 4; b++) {
      if (menuButtons[b].justPressed()) {
        if (b == 3)  { // Menu
          tft.setCursor(0, 0);
          tft.fillScreen(BLACK);
          Serial.println("menu");
          menuView();
          break;
        } else if (b == 2) { // Annunciate
          tft.setCursor(0, 0);
          tft.fillScreen(BLACK);
          // Setting flags in schedule
          // First measure everything, then annunciate everything
          //measurementSelection = 3; // always 3
          // addFlags[0] = 1;
          //addFlags[1] = 1;
          addFlags[3] = 1;
          break;
        }
      }
    }
    delay(100);
  }
}

void appendAtEnd(TCB * newNode) {
  TCB* temp = linkedListHead;
  newNode->next = NULL;

  // if empty
  if (linkedListHead == NULL) {
    newNode->prev = NULL;
    linkedListHead = newNode;
    return;
  }

  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = newNode;
  newNode->prev = temp;

  return;
}

void deleteNode(TCB * delNode) {

  // if NULL delNode
  if (linkedListHead == NULL || delNode == NULL)
    return;

  // if node is head node
  if (linkedListHead == delNode) {
    linkedListHead = delNode->next;
  }

  // if node to be deleted is not the last node
  if (delNode->next != NULL) {
    delNode->next->prev = delNode->prev;
  }
  if (delNode->prev != NULL) {
    delNode->prev->next = delNode->next;
  }
  free(delNode);
  return;
}

void updateTask(int ID, bool insert) {
  switch (ID) {
    case 0:
      if (insert) {
        appendAtEnd(&MeasureTask);
        Serial.println("MeasureTask added");
      } else {
        deleteNode(&MeasureTask);
        Serial.println("MeasureTask deleted");
      }
      break;
    case 1:
      if (insert) {
        appendAtEnd(&ComputeTask);
        Serial.println("ComputeTask added");
      } else {
        deleteNode(&ComputeTask);
        Serial.println("ComputeTask deleted");
      }
      break;
    case 2:
      if (insert) {
        appendAtEnd(&DisplayTask);
        Serial.println("DisplayTask added");
      } else {
        deleteNode(&DisplayTask);
        Serial.println("DisplayTask deleted");
      }
      break;
    case 3:
      if (insert) {
        appendAtEnd(&AnnunciateTask);
        Serial.println("AnnunciateTask added");
      } else {
        deleteNode(&AnnunciateTask);
        Serial.println("AnnunciateTask deleted");
      }
      break;
    case 4:
      if (insert) {
        appendAtEnd(&StatusTask);
        Serial.println("StatusTask added");
      } else {
        deleteNode(&StatusTask);
        Serial.println("StatusTask deleted");
      }
      break;
    case 5:
      if (insert) {
        appendAtEnd(&KeypadTask);
        Serial.println("KeypadTask added");
      } else {
        deleteNode(&KeypadTask);
        Serial.println("KeypadTask deleted");
      }
      break;
  }
}

void loop(void) {
  scheduler();
}

void scheduler() {
  measurementSelection = 3;
  updateTask(0, true);
  TCB* temp = linkedListHead;
  /*for(int i = 0; i < 6; i++) {
        if(addFlags[i]) {
            updateTask(i, true); // insert task
            addFlags[i] = 0;
            removeFlags[i] = 1; // Schedule removal when done
        }
        if(removeFlags[i]) {
  		updateTask(i, false);
  		removeFlags[i] = 0;
  	}
    }*/
  while (temp != NULL) {
    (*temp->taskFuncPtr)(temp->taskDataPtr);
    Serial.println("Task Running");
    temp = temp->next;
  }
  for (int i = 0; i < 6; i++) {
    if (removeFlags[i]) {
      updateTask(i, false);
      removeFlags[i] = 0;
    }
  }
}

void setup(void) {

  //for communication with uno
  Serial1.begin(9600);

  // for TimerThree
  Timer3.initialize(500000);
  Timer3.attachInterrupt(updateCounter);


  // for display
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

  // Might need to check these
  // Measure
  MeasureTaskData measure;
  measureData = measure;
  measure.temperatureRawPtr = temperatureRawBuf;
  measure.bloodPressRawPtr = bloodPressRawBuf;
  measure.pulseRateRawPtr = pulseRateRawBuf;
  measure.measurementSelectionPtr = &measurementSelection;

  TCB Measure;
  Measure.taskFuncPtr = &measureDataFunc;
  Measure.taskDataPtr = &measureData;
  Measure.next = NULL;
  Measure.prev = NULL;
  MeasureTask = Measure;

  // Compute
  ComputeTaskData compute;
  computeData = compute;
  compute.temperatureRawPtr = temperatureRawBuf ;
  compute.bloodPressRawPtr = bloodPressRawBuf ;
  compute.pulseRateRawPtr = pulseRateRawBuf ;
  compute.measurementSelectionPtr = &measurementSelection ;
  compute.temperatureCorrectedPtr = temperatureCorrectedBuf;
  compute.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
  compute.pulseRateCorrectedPtr = pulseRateCorrectedBuf;

  TCB Compute;
  Compute.taskFuncPtr = &computeDataFunc ;
  Compute.taskDataPtr = &computeData ;
  Compute.next = NULL;
  Compute.prev = NULL;
  ComputeTask = Compute;

  // Display
  DisplayTaskData display;
  displayData = display;
  display.temperatureCorrectedPtr = temperatureCorrectedBuf;
  display.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
  display.pulseRateCorrectedPtr = pulseRateCorrectedBuf;
  display.batteryStatePtr = &batteryState;

  TCB Display;
  Display.taskFuncPtr = &displayDataFunc ;
  Display.taskDataPtr = &displayData ;
  Display.next = NULL;
  Display.prev = NULL;
  DisplayTask = Display;

  // Warning
  WarningTaskData warning;
  warningData = warning;
  warning.temperatureCorrectedPtr = temperatureCorrectedBuf;
  warning.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
  warning.pulseRateCorrectedPtr = pulseRateCorrectedBuf;
  warning.batteryStatePtr = &batteryState;

  TCB Annunciate;
  Annunciate.taskFuncPtr = &annunciateDataFunc ;
  Annunciate.taskDataPtr = &warningData ;
  Annunciate.next = NULL;
  Annunciate.prev = NULL;
  AnnunciateTask = Annunciate;

  // Status
  StatusTaskData status;
  statusData = status;
  status.batteryStatePtr = &batteryState;

  TCB Status;
  Status.taskFuncPtr = &statusDataFunc;
  Status.taskDataPtr = &statusData ;
  Status.next = NULL;
  Status.prev = NULL;
  StatusTask = Status;

  // Keypad
  KeypadTaskData keypad;
  keypadData = keypad;
  keypad.measurementSelectionPtr = &measurementSelection;
  keypad.alarmAcknowledgePtr = &alarmAcknowledge;

  TCB Keypad;
  Keypad.taskFuncPtr = &keypadDataFunc ;
  Keypad.taskDataPtr = &keypadData ;
  Keypad.next = NULL;
  Keypad.prev = NULL;
  KeypadTask = Keypad;

  // Communication
  CommunicationTaskData comm;
  commData = comm;
  comm.temperatureCorrectedPtr = temperatureCorrectedBuf;
  comm.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
  comm.pulseRateCorrectedPtr = pulseRateCorrectedBuf;

  /* Should happen everytime so no need make it a task
    TCB Comm;
    Comm.taskFuncPtr = &commDataFunc ;
    Comm.taskDataPtr = &commData ;
    Comm.next = NULL;
    Comm.prev = NULL;
    CommTask = Comm;
  */

  // NULL TCB
  TCB Nulll;
  Nulll.taskFuncPtr = NULL;
  Nulll.taskDataPtr = NULL;
  Nulll.next = NULL;
  Nulll.prev = NULL;
  NullTask = Nulll;

  // initial values
  bpHigh = false;
  tempHigh = false;
  pulseLow = false;
  battLow = false;

}
