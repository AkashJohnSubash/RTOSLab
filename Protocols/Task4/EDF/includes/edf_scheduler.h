#include "freertos/FreeRTOS.h"      // Kernel headers
#include "freertos/task.h"
#include "freertos/list.h"

#ifndef EDF_SCHEDULER_H_
#define EDF_SCHEDULER_H_

//---------------------defines----------------------------
#define LOWEST_SYS_PRIO                     tskIDLE_PRIORITY
#define MAX_SYS_PRIO                        configMAX_PRIORITIES - 5
#define LOCAL_STORAGE_INDEX                 0

#define IDLE_TASK_NUM 0
#define SCHED_TASK_NUM 1
#define MAIN_TASK_NUM 2
#define STARTUP_DELAY_MS 200
#define STARTUP_DELAY_TICK pdMS_TO_TICKS(STARTUP_DELAY_MS)

#define LOG_NUM_EVNTS 50
#define LOG_NUM_FIELDS 4
#define LOG_TASK_IDX 0
#define LOG_ENTRY_IDX 1
#define LOG_EXIT_IDX 2
#define LOG_OVRFLW_IDX 3

#define TRACE_TIMER_TIMEOUT                 5 * 1000 * 1000

#define Us_TO_TICKS( xTimeInUs ) (( xTimeInUs ) /  10000U)

// --------------------- data structures--------------------
extern BaseType_t firstTaskUs;
extern TickType_t sysStartTick;

// User specified public TCB
typedef struct pubTCB
{   
    TickType_t  periodMs;                         // periodic task period
    TickType_t  phaseMs;                          // periodic task phase
    BaseType_t  WcetMs;                           // Wort case execution time estimate
    TickType_t 	relDeadlineMs;  
    BaseType_t  taskNumber; 
    BaseType_t  isAperiodic; 
    BaseType_t 	relArrivalMs; 
    void  (*taskCallbackFunction)(void*);
   }pubTCB_t;

// --------------- public function prototypes ------------------

BaseType_t EdfScheduleInit(pubTCB_t* pTask_arr, pubTCB_t* apTask_arr);
void EdfScheduleStart();
void EdfDeleteAllTasks();
void busyWaitLoop(int computeUs);

#endif /* EDF_SCHEDULER_H_ */