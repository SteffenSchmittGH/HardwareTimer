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

/* [] END OF FILE */
#include "myTracer.h"
#include "UART_LOG.h"
#include <stdlib.h>
#include "project.h"


/********************************************************************************************************/
#include "Pin_1.h" 
/* Pin Related Registers */
#define Pin_1_DR                 (* (reg8 *) Pin_1__DR)
#define Pin_1_MASK               Pin_1__MASK
#define Pin_1_SHIFT              Pin_1__SHIFT
#define SET_PIN 1
#define CLEAR_PIN 0
/*-----------------------*/
#define TIMER_STOP_MASK 0xFFFFFFFE
/*Data WatchPoint Unit related Registers*/
#define MY_DWT_CTRL CYDEV_DWT_CTRL //0xE0001000
#define MY_DWT_CCNT CYDEV_DWT_CYCLE_COUNT //0xE0001004 
#define DEMCR_MY 0xE000EDFC
/*-------------------------------------*/
/*SYSTICK START VALUE*/
#define SYSTICK_START_VALUE 16777215

void (*systickHandlerPtr)(void); //for NVIC function pointer to the below Handler function
volatile uint32_t systickCount = 0;
/********************************************************************************************************/
boolean_t globalSYSTICKActive = 0;
boolean_t globalDWTActive = 0;
boolean_t globalPINActive = 0;

void SysTick_Handler(void){
    systickCount++;  
}

void myTimer_init(myTracer_t* me,  tracerMode_t mode, char* name){
    UART_LOG_Start();
    me->m_mode = mode;
    me->current_time = 0;
    me->m_name = malloc(strlen(name));
    me->m_name = name;
    me->m_status = INITIALIZED;
    if (me->m_mode == SYSTICK && globalSYSTICKActive == 0){
        globalSYSTICKActive = 1;
        sprintf(me->m_name,"%s","SysTick");
        //Disable DWT and PIN
        systickHandlerPtr = SysTick_Handler;
        CY_SYS_SYST_CSR_REG = 0x00;
        CY_SYS_SYST_RVR_REG = 0x5DC0;//SysTick_LOAD_RELOAD_Msk;
        NVIC_SetPriority(SysTick_IRQn, 7);
        NVIC_SetVector(SysTick_IRQn, (uint32_t)systickHandlerPtr);
        CY_SYS_SYST_CVR_REG = 0;
        CY_SYS_SYST_CSR_REG |= SysTick_CTRL_CLKSOURCE_Msk;
        CY_SYS_SYST_CSR_REG |= SysTick_CTRL_TICKINT_Msk;
    }else if(me->m_mode == MyDataWatchPoint && globalDWTActive == 0){
        globalDWTActive = 1;
        sprintf(me->m_name,"%s","DataWatchPointTimer");
        //Disable SYSTICK and PIN
        CY_SYS_SYST_CSR_REG &= 0xFFFFFFFE;
        /*Enable the DWT*/
        uint32_t* myRegPtr;
        myRegPtr = (uint32_t*)  MY_DWT_CTRL;
        *myRegPtr &= 0xFFC00000; //First Bits are read only. From 22 they are writable.
        uint32_t* demcrREG = (uint32_t*)DEMCR_MY;
        *demcrREG |= CoreDebug_DEMCR_TRCENA_Msk;
    }else if(me->m_mode == PIN&& globalPINActive == 0){ //If this case, boolean pin is always true!
        globalPINActive = 1;
        //Disable DWT and SYSTICK
        sprintf(me->m_name,"%s","MEASURE VIA OSZILLOSCOP");
    }else{
        UART_LOG_PutString("ERROR: This type doesnt exist!\n"); 
    }
    return;
}

void myTimer_start(myTracer_t* me){
    if(me->m_status != INITIALIZED || me->m_status != PAUSED || me->m_status != RESETTED){
        return;   
    }
    if(me->m_mode == SYSTICK){
        systickCount = me->current_time;
        CY_SYS_SYST_CSR_REG |= SysTick_CTRL_ENABLE_Msk;
    }else if(me->m_mode == MyDataWatchPoint){
        *(uint32_t*)MY_DWT_CCNT = me->current_time;
        uint32_t* dwt_ctrl_register;
        dwt_ctrl_register = (uint32_t*)MY_DWT_CTRL;
        *dwt_ctrl_register |= 0x01; //Starts Cycle Count
    }else{// sets the pin
        uint8 staticBits = (Pin_1_DR & (uint8)(~Pin_1_MASK));
        Pin_1_DR = staticBits | ((uint8)(SET_PIN << Pin_1_SHIFT) & Pin_1_MASK);  
    }
}

uint32_t myTimer_getCurrentValue(myTracer_t* me){
    return me->current_time;  
}

float myTimer_CalculateElapsedTimeInMS(myTracer_t* me){
    if(me->m_mode == SYSTICK){
        return (float) systickCount;
    }else if(me->m_mode == MyDataWatchPoint){
        uint32_t* fetchCountValue;
        fetchCountValue = (uint32_t*)MY_DWT_CCNT;
        return ((float)((float)*fetchCountValue*4.16666667e-5));
    }else{
        UART_LOG_PutString("There is not Calculated Time for chosen Timer mode");  
        return 0.f;
    }
}

void myTimerStop(myTracer_t* me){
    if(me->m_mode == SYSTICK){
        CY_SYS_SYST_CSR_REG &= 0xFFFFFFFE;
        me->current_time = systickCount;
        systickCount = 0;
    }else if(me->m_mode == MyDataWatchPoint){
        uint32_t* dwt_ctrl_ptr = (uint32_t*)MY_DWT_CTRL;
        *dwt_ctrl_ptr &= 0xFFFFFFFE;
        me->current_time = *(uint32_t*)MY_DWT_CCNT;
        *(uint32_t*)MY_DWT_CCNT &= 0x00;
    }
    me->m_status = PAUSED;
    return;
}
/*KEin unterschied zu start kann vllt gelöscht werden*/
void myTimerResume(myTracer_t* me){
    if(me->m_mode == SYSTICK){
        systickCount = me->current_time;
        CY_SYS_SYST_CSR_REG |= SysTick_CTRL_ENABLE_Msk;
    }else if(me->m_mode == MyDataWatchPoint){
        uint32_t* dwt_ctrl_ptr = (uint32_t*)MY_DWT_CTRL;
        uint32_t* dwt_cnt_ptr = (uint32_t*)MY_DWT_CCNT;
        *dwt_cnt_ptr = me->current_time;
        *dwt_ctrl_ptr |= 0x01;
    }
    return;
}

void myTimerReset(myTracer_t* me){
    if(me->m_mode == SYSTICK){
        me->current_time = 0;
        systickCount = 0;
    }else if(me->m_mode == MyDataWatchPoint){
        uint32_t* dwt_ctrl_ptr = (uint32_t*)MY_DWT_CTRL;
        uint32_t* dwt_cnt_ptr = (uint32_t*)MY_DWT_CCNT;
        *dwt_ctrl_ptr &= TIMER_STOP_MASK;
        me->current_time = 0;
        *dwt_cnt_ptr &= 0x00;
    }
    me->m_status = RESETTED;
   return; 
}

void myTimer_print(myTracer_t* me){
    char buffer[100];
    if(me->m_mode == SYSTICK){
        snprintf(buffer, sizeof(buffer), "Elapsed Time Systick %s : %lu ms \r\n",me->m_name, me->current_time);
    }else if(me->m_mode == MyDataWatchPoint){
        uint32_t* CNT_Value;
        CNT_Value = (uint32_t*)MY_DWT_CCNT;
        float dwtCNT = (float)(*CNT_Value)*4.16666667e-5;
        snprintf(buffer, sizeof(buffer), "DWT %s Elapsed CNT Register: %lu and elapsed time in ms: %.6f \r\n",me->m_name, MY_DWT_CCNT, dwtCNT);
    }else{
        uint32_t* CNT_Value;
        *CNT_Value = 0;
        snprintf(buffer, sizeof(buffer), "Elapsed Time: %lu, due to PIN measurement.\r\n", 0);
    }
    UART_LOG_PutString(buffer);
    return;
}