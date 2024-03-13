#include <string.h>                 // Standard lib headers
#include <stdint.h>
#include <stdio.h>
#include "application.h"            // Application headers
#include "periodic_schedule.h"      
#include "freertos/FreeRTOS.h"      // Kernel headers
#include "freertos/task.h"          
#include "esp_err.h"

static int periodic_task_ctr = 0;
int app_idx = 0;

// Wrapper function to start periodic tasks
int16_t run_periodic_tasks(pTaskParameters_t* pTask_arr)
{   
    //  all  initialized periodic tasks will be scheduled
    printf("[INFO] Periodic application task scheduler invoked\r"); // Necessary to generate sufficient init delay
    int16_t ret_val = ESP_FAIL; 
    // struct pTaskParameters_t* pTaskArr_pst[APP_NUM] = ((struct pTaskParameters_t*)pTask_arr[APP_NUM]);
    char task_name[configMAX_TASK_NAME_LEN];
    pTaskParameters_t pTask;

    periodic_task_ctr++;
    for (int16_t i = 0; i < APP_PER_NUM; i++) 
    {  
        snprintf(task_name, configMAX_TASK_NAME_LEN, "%d_Appl", periodic_task_ctr);
        // memcpy(pTask.taskName, task_name, sizeof(pTask.taskName));
        // Period, phase, prio can only be configured before task startup
        pTask.callback = pTask_arr[i].callback;
        pTask.periodMs = pTask_arr[i].periodMs;
        pTask.phaseMs = pTask_arr[i].phaseMs;
        // Allow only task priorities greater than nominal value
        if(pTask_arr[i].priority >= APP_BASE_PRIO)
        {
            pTask.priority = pTask_arr[i].priority;
        }
        else
        {
            pTask.priority = APP_BASE_PRIO;
            printf("WARNING : Priority invalid for task %d", i);
        }
        
        xTaskCreate(pTask.callback, task_name, 4096, (void*) &pTask, pTask.priority, NULL);
    }
    ret_val = ESP_OK;

    return ret_val;
}



