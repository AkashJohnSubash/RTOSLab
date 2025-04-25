#include "application.h"            // Application headers
#include "edf_scheduler.h"

// -------------------Global variales------------------------
int idx_gl = 0;
int taskLog[LOG_NUM_EVNTS][LOG_NUM_FIELDS] = {0};

// --------------------Private function prototypes--------------------

void prvRecordStartTimeTrace(BaseType_t xTaskNumber, BaseType_t xTCBNumber);
void prvRecordEndTimeTrace(BaseType_t xTaskNumber, BaseType_t xTCBNumber);
void displ_task(void* Unused);

// --------------------Function definitions-----------------------------

/* Function invoked by kernel via traceTASK_SWITCHED_IN() macro
   to log task instance switch in time */
void prvRecordStartTimeTrace(BaseType_t xTaskNumber, BaseType_t xTCBNumber)
{
	if(idx_gl < LOG_NUM_EVNTS)
	{
		taskLog[idx_gl][0] = xTaskNumber;
		taskLog[idx_gl][1] = esp_timer_get_time();
	}
}

/* Function invoked by kernel via traceTASK_SWITCHED_OUT() macro
   to log task instance switch in time */
void prvRecordEndTimeTrace(BaseType_t xTaskNumber, BaseType_t xTCBNumber)
{
	if(idx_gl < LOG_NUM_EVNTS)
	{
		taskLog[idx_gl][2] = esp_timer_get_time();
		idx_gl++;
	}
}

void logToSerial(pubTCB_t* pTask_arr, pubTCB_t* apTask_arr)
{
    printf("\n [DEBUG] Metadata start");
    printf("\n [DEBUG] Task start time Us : %d", firstTaskUs);
    printf("\n [DEBUG] System start tick %d", sysStartTick);

    for (int16_t i = 0; i < APP_PER_TASKS; i++)
    {
        // print aperiodic meta data for plot
        printf("\n Periodic Task : %d, arrival : %d, phase : %d, wcet : %d, deadline : %d",
        pTask_arr[i].taskNumber,
        0,
        pTask_arr[i].phaseMs,
        pTask_arr[i].WcetMs,
        pTask_arr[i].relDeadlineMs);
    }

    for (int16_t i = 0; i < APP_APER_TASKS; i++)
    {
        // print periodic metadata for plot
        printf("\n Aperiodic Task : %d arrival : %d, phase : %d, wcet : %d, deadline : %d",
        apTask_arr[i].taskNumber,
        apTask_arr[i].relArrivalMs,
        0,
        apTask_arr[i].WcetMs,
        apTask_arr[i].relDeadlineMs);
    }
    printf("\n[DEBUG] Metadata end\n");
    xTaskCreate(displ_task, "DisplayTask", 4096u, NULL, MAX_SYS_PRIO, NULL);
}


/* Print fixed number of tasks to serial output or file*/
void displ_task(void* Unused)
{
    while(1)
    {
        //Display buffer only once, when filled
        if(idx_gl == LOG_NUM_EVNTS)
        {
            printf("********\n");
            printf("\n[INFO] Schedule Data Gathered. Printing data......\n");
            for(int i = 0; i < LOG_NUM_EVNTS; i++)
                printf("\nEvent No: %d, Task No: %d, Entry at: %d Us, Exit at: %d Us. Overflow at %d Us",
                 i,
                 taskLog[i][LOG_TASK_IDX],
                 taskLog[i][LOG_ENTRY_IDX],
                 taskLog[i][LOG_EXIT_IDX],
                 taskLog[i][LOG_OVRFLW_IDX]);
            printf("\n[INFO] Schedule printed!!!\n");
            printf("\n********");
        }
        // Delete display task once complete
		vTaskDelete(NULL);
    }
}