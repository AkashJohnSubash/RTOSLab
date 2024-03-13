#include <stdio.h>                  // Standard lib headers
#include "application.h"            // Application headers
#include "periodic_schedule.h"
#include "freertos/FreeRTOS.h"      // Kernel headers
#include "freertos/task.h" 

void app_main(void)
{   
    pTaskParameters_t pTask_arr[APP_PER_NUM] = {{0}};
    // Generate init delay
    printf("[INFO] Application entry point found\r");
    vTaskDelay(pdMS_TO_TICKS(100u));
    // All tasks with same priority
    reg_periodic_tasks(pTask_arr);
    run_periodic_tasks(pTask_arr);
    xTaskCreate(displ_task, "DisplayTask", 4096u, NULL, DISPL_PRIO, NULL);
}

// ------------------------Application task definitions-----------------------
int16_t reg_periodic_tasks(pTaskParameters_t* pTask_arr)
{    
    // register the periodic tasks you intend to execute

    pTask_arr[0].callback = app_pTask1;
    pTask_arr[0].periodMs = 100;
    pTask_arr[0].priority = 2;

    pTask_arr[1].callback = app_pTask2;
    pTask_arr[1].periodMs = 100;
    pTask_arr[1].priority = 2;

    pTask_arr[2].callback = app_pTask3;
    pTask_arr[2].periodMs = 150;
    pTask_arr[2].priority = 2;
    
    return ESP_OK;
}

// periodic task1
void app_pTask1(void* pTsk_pst)
{   
    TickType_t xLastWakeTime;
    pTaskParameters_t* param_pst= ((pTaskParameters_t *)pTsk_pst);
    const TickType_t xPeriod =  pdMS_TO_TICKS(param_pst->periodMs);

    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // Task exection time
        for (int i = 0; i < (APP_COMP_ITER); i++) 
        {
            // simulates minimal computation time
            __asm__ __volatile__("NOP");
        }
        // Block task until next execution at xLastWakeTime + xPeriod.
        vTaskDelayUntil( &xLastWakeTime, xPeriod);
    // Add user application code below this line
    }
}

// periodic task2
void app_pTask2(void* pTsk_pst)
{   
    TickType_t xLastWakeTime;
    pTaskParameters_t* param_pst= ((pTaskParameters_t *)pTsk_pst);
    const TickType_t xPeriod =  pdMS_TO_TICKS(param_pst->periodMs);

    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // Task exection time
        for (int i = 0; i < (APP_COMP_ITER); i++) 
        {
            // 4 times minimal execution time
            __asm__ __volatile__("NOP");
        }
        // Block task until next execution at xLastWakeTime + xPeriod.
        vTaskDelayUntil( &xLastWakeTime, xPeriod);
    // Add user application code below this line
    }
}

// periodic task3
void app_pTask3(void* pTsk_pst)
{   
    TickType_t xLastWakeTime;
    pTaskParameters_t* param_pst = ((pTaskParameters_t *)pTsk_pst);
    const TickType_t xPeriod =  pdMS_TO_TICKS(param_pst->periodMs);

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // Task exection time
        for (int i = 0; i < (APP_COMP_ITER ); i++) 
        {
            // 6 times minimal coputation time
            __asm__ __volatile__("NOP");
        }
        // Block task until next execution at xLastWakeTime + xPeriod.
        vTaskDelayUntil( &xLastWakeTime, xPeriod);
    // Add user application code below this line
    }
}


int idx_gl = 0;
//#define TASK_BUF_LEN  100
//int taskList_size = 100;
int taskList[TASK_BUF_LEN][2] = {0}; 
void displ_task(void* Unused)
{   
    while(1)
    {    
        //Display buffer only once, when filled
        if(idx_gl == TASK_BUF_LEN)
        { 
            printf("[INFO] Schedule Data Gathered. Printing data......\n");
            for(int i = 0; i < TASK_BUF_LEN; i++)
                printf("Tick %d: %d\n", taskList[i][1], taskList[i][0]);
            printf("\n[INFO] Schedule printed!!!\n");
            idx_gl++;                       
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}