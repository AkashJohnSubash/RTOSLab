#include <stdio.h>                  // Standard lib headers
#include "application.h"            // Application headers
#include "freertos/FreeRTOS.h"      // Kernel headers
#include "freertos/task.h"
#include <rom/ets_sys.h>

// -----------------------------Function declarations--------------------------
void app_main(void)
{
    // Generate init delay
    printf("\n[INFO] Application entry point found\r");
    pubTCB_t pTask_arr[APP_PER_TASKS] = {{0}};
    pubTCB_t apTask_arr[APP_APER_TASKS] = {{0}};
    // user configures public periodic and aperiodic TCB
    reg_periodic_tasks(pTask_arr);
    reg_aperiodic_tasks(apTask_arr);
    // initialize the EDF scheduler
    EdfScheduleInit(pTask_arr, apTask_arr);
    EdfScheduleStart();
    // collect schedule and then delete tasks
    vTaskDelay(pdMS_TO_TICKS(4000));
    EdfDeleteAllTasks();
    logToSerial(pTask_arr, apTask_arr);

}

// ------------------------User function definitions-----------------------

BaseType_t reg_periodic_tasks(pubTCB_t* pTask_arr)
{
    // register the public periodic TCB
    pTask_arr[0].phaseMs = 100;
    pTask_arr[0].WcetMs = 100;
    pTask_arr[0].periodMs = 400;
    pTask_arr[0].relDeadlineMs = 400;
    pTask_arr[0].taskNumber = 5;
    pTask_arr[0].taskCallbackFunction = app_pTask1;

    pTask_arr[1].phaseMs = 0;
    pTask_arr[1].WcetMs = 200;
    pTask_arr[1].periodMs = 800;
    pTask_arr[1].relDeadlineMs = 800;
	pTask_arr[1].taskNumber = 6;
    pTask_arr[1].taskCallbackFunction = app_pTask2;

    pTask_arr[2].phaseMs = 200;
    pTask_arr[2].WcetMs = 100;
    pTask_arr[2].periodMs = 400;
    pTask_arr[2].relDeadlineMs = 400;
	pTask_arr[2].taskNumber = 7;
    pTask_arr[2].taskCallbackFunction = app_pTask3;

    return ESP_OK;
}

BaseType_t reg_aperiodic_tasks(pubTCB_t* apTask_arr)
{
    // register the public aperiodic TCB
    apTask_arr[0].WcetMs = 100;
    apTask_arr[0].relArrivalMs = 200;
	apTask_arr[0].taskNumber = 14;
    apTask_arr[0].taskCallbackFunction = app_apTask1;

    apTask_arr[1].WcetMs = 100;
    apTask_arr[1].relArrivalMs = 800;
	apTask_arr[1].taskNumber = 15;
    apTask_arr[1].taskCallbackFunction = app_apTask2;

    return ESP_OK;
}

// periodic task1
void app_pTask1(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// periodic task2
void app_pTask2(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// periodic task3
void app_pTask3(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// periodic task4
void app_pTask4(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// aperiodic task1
void app_apTask1(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// aperiodic task2
void app_apTask2(int WcetUs)
{
    // Replace busy waiting function with user code
    busyWaitLoop(WcetUs);
}

// keep CPU busy for the WCET duration
void busyWaitLoop(int WcetUs)
{
    // split into smaller chunks for preemtion to work correctly
    for(int i = 0; i < NUM_SLICES; i++)
    {
        ets_delay_us((WcetUs)/NUM_SLICES);
    }
}