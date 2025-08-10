/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "global.h"
#include <stdio.h>
#define MAX_TIMERS 5

typedef enum {
    RUNNING,
    INITIALIZED,
    PAUSED,
    RESETTED,
    DEINITIALIZED,
}tracerStatus_t;

typedef enum {
    SYSTICK,
    MyDataWatchPoint,
    PIN
}tracerMode_t;

typedef struct {
    tracerMode_t m_mode; //SYSTICK,DWT,PIN
    char* m_name;
    uint32_t current_time;
    tracerStatus_t m_status;
}myTracer_t;

void myTimer_init(myTracer_t* me, tracerMode_t mode, char* name);
void myTimer_start(myTracer_t* me);

void myTimerStop(myTracer_t* me);
void myTimerResume(myTracer_t* me);
void myTimerReset(myTracer_t* me);
uint32_t myTimer_getCurrentValue(myTracer_t* me);
float myTimer_CalculateElapsedTimeInMS(myTracer_t* me);
void myTimer_print(myTracer_t* me);
/* [] END OF FILE */
