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
#include "project.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "myTracer.h"
uint32_t calculations = 0;
int main(void)
{
    CyGlobalIntEnable;  /* Enable global interrupts. */
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    myTracer_t myTracerObj1;
    myTracer_t myTracerObj2;
    myTimer_init(&myTracerObj1, MyDataWatchPoint,"Timer1");
    myTimer_init(&myTracerObj2, MyDataWatchPoint,"Timer2");
    UART_LOG_PutString("Start Program \r\n");
    myTimer_start(&myTracerObj1);
    myTimer_start(&myTracerObj2);
    for(int i = 0; i < 1000; i++){
        calculations = sqrt(i);
    }
    myTimerStop(&myTracerObj1);
    myTimerStop(&myTracerObj2);
    myTimer_print(&myTracerObj1);
    myTimer_print(&myTracerObj2);
    myTimerReset(&myTracerObj1);
    myTimerResume(&myTracerObj1);
    for(int i = 0; i < 1000; i++){
        calculations = sqrt(i);    
    }
    myTimerStop(&myTracerObj1);
    UART_LOG_PutString("\r\n");
    myTimer_print(&myTracerObj1);
    UART_LOG_PutString("\r\n");
    UART_LOG_PutString("End Program \r\n");
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
