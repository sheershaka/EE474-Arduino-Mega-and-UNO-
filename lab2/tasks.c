#include <stdio.h>
#include "booltype.c"

unsigned int temperatureRaw;
unsigned int systolicPressRaw;
unsigned int diastolicPressRaw;
unsigned int pulseRateRaw;

unsigned char* tempCorrected;
unsigned char* systolicPressCorrected;
unsigned char* diastolicPressCorrected;
unsigned char* pulseRateCorrected;

unsigned short batteryState;

unsigned char bpOutOfRange;
unsigned char tempOutOfRange;
unsigned char pulseOutOfRange;

Bool bpHigh;
Bool tempHigh;
Bool pulseLow;

void measure(void* data);
void Compute(void* data);
void Warning_Alarm(void* data);
void Status(void* data);

struct MeasureData
{
    unsigned int* tempRaw;
    unsigned int* sysRaw;
    unsigned int* diasRaw;
    unsigned int* pulseRaw;
};

struct ComputeData
{
    unsigned int* tempRaw;
    unsigned int* sysRaw;
    unsigned int* diasRaw;
    unsigned int* pulseRaw;
    unsigned char* tempCor;
    unsigned char* sysCor;
    unsigned char* diasCor;
    unsigned char* prCor;
};

struct DisplayData
{
    unsigned char* tempCor;
    unsigned char* sysCor;
    unsigned char* diasCor;
    unsigned char* prCor;
    unsigned short* batState;
};

struct WarningAlarmData
{
    unsigned int* tempRaw;
    unsigned int* sysRaw;
    unsigned int* diasRaw;
    unsigned int* pulseRaw;
    unsigned short* batState;
};

struct Status
{
    unsigned short* batState;
};

struct SchedulerData
{
    void* nothing;
};

void measure(void* data)
{
    (struct MeasureData*) data;
    (*data).tempRaw = &temperatureRaw;
    (*data).sysRaw  = &systolicPressRaw;
    (*data).diasRaw = &diastolicPressRaw;
    (*data).pulseRaw= &pulseRateRaw;

    Bool sys = FALSE;
    Bool dias = FALSE;
    int tempA = 2;
    int tempB = -1;
    int pulseA = -1;
    int pulseB =  3;

    // Set changes for temperature
    if ((*data).tempRaw > 50)
    {
        tempA = -1;
        tempB = 2;
    }
    else if ((*data).tempRaw < 15)
    {
        tempA = 2;
        tempB = -1;
    };

    //Set changes for pulse
    if ((*data).pulseRaw > 40)
    {
        pulseA = 3;
        pulseB = -1;
    }
    else if ((*data).pulseRaw < 15)
    {
        pulseA = -1;
        pulseB = 3;
    }

    // Check systolic and diastolic activation
    if ((*data).sysRaw > 100)
        sys = TRUE;
    if ((*data).diasRaw < 40)
        dias = TRUE;

    if (dias)
        sys = FALSE;
    if (sys)
        dias = FALSE;

    // If time counter is even
    //if (time_base)
    //{
        (*data).tempRaw += tempA;

        if (!sys)
            (*data).sysRaw  += 3;

        if (!dias)
            (*data).diasRaw += 2;

        (*data).pulseRaw+= pulseA;
    //};
    //else
    //{
        (*data).tempRaw += tempB;

        if (!sys)
            (*data).sysRaw  -= 1;

        if (!dias)
            (*data).diasRaw -= 1;

        (*data).pulseRaw+= pulseB;
    //};
};

void Compute(void* data)
{
    (struct ComputeData*) data;
    (*data).tempRaw  = &temperatureRaw;
    (*data).sysRaw   = &systolicPressRaw;
    (*data).diasRaw  = &diastolicPressRaw;
    (*data).pulseRaw = &pulseRateRaw;
    (*data).tempCor  = &tempCorrected;
    (*data).sysCor   = &systolicPressCorrected;
    (*data).diasCor  = &diastolicPressCorrected;
    (*data).prCor    = &pulseRateCorrected;

    (*data).tempCor  = 5 + 0.75 * (*data).tempRaw;
    (*data).sysCor   = 9 + 2    * (*data).sysRaw;
    (*data).diasCor  = 6 + 1.50 * (*data).diasRaw;
    (*data).prCor    = 8 + 3    * (*data).pulseRaw;
};

void Warning_Alarm(void* data)
{
    (struct WarningAlarmData*) data;
    (*data).tempRaw  = &temperatureRaw;
    (*data).sysRaw   = &systolicPressRaw;
    (*data).diasRaw  = &diastolicPressRaw;
    (*data).pulseRaw = &pulseRateRaw;
    (*data).batState = &batteryState;

};

void Status(void* data)
{
    (struct Status*) data;
    (*data).batState = &batteryState;

    (*data).batState -= 1;
};
