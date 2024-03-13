#include <stdio.h>
#include <math.h>
#include "application.h" // Application headers
#include "edf_scheduler.h"
#include "freertos/FreeRTOS.h" // Kernel headers
#include "freertos/task.h"
#include "esp_err.h"

// -------------------Private data structures ---------------------------
//  extended TCB
typedef struct extTCB
{
	TickType_t periodTick; // periodic task period
	TickType_t phaseTick;
	BaseType_t WcetUs;
	TickType_t relArrivalTick;
	TickType_t relDeadlineTick;
	TickType_t absDeadlineTick;
	BaseType_t isAperiodic;
	BaseType_t toDelete;
	BaseType_t priority; // periodic task priority
	BaseType_t taskNumber;
	TaskHandle_t taskHandle;
	ListItem_t xTCBListItem; // Implmentation specific param
	BaseType_t phaseShifted; // implmentation specific param
	BaseType_t executed;
	void (*taskCallbackFunction)(void *);
} exTCB_t;

// -------------------Global variables------------------------

BaseType_t firstTaskUs = 0;
TickType_t sysStartTick = 0;

static BaseType_t TasksCreated = pdFALSE;
static float periodicUtil = 0;
static float aperiodicUtil = 1e-15; // Default indicating no aperiodic scheduling bandwidth
BaseType_t numOfTasks = 0;

static TaskHandle_t mainTaskHandle = NULL;
static TaskHandle_t idleTaskHandle = NULL;

// Simplify periodic list vars
static List_t xTCBListMainVar;
static List_t xTCBListTempVar;
static List_t *xTCBListMain = &xTCBListMainVar;
static List_t *xTCBListTemp = &xTCBListTempVar;

// --------------------Private function prototypes--------------------

static BaseType_t prvInit_periodic_tasks(pubTCB_t *pTask_arr);
static void prvSwapLists(List_t **aTCBList, List_t **bTCBList);
static void prvDeleteTCBFromList(exTCB_t *xTCB);
static float prvMaximum(float num1f, float num2f);

void prvEdfUpdatePrioritiesTrace(void);
void prvEdfCheckDeadlineTrace(void);
static void prvEdfTaskWrapper(void *pvParameters);

// --------------------Function definitions-----------------------------

// Fully initialize the aperiodic TCB  and insert it the Queue
BaseType_t prvInit_aperiodic_tasks(pubTCB_t *apTask_arr)
{
	//  all  initialized periodic tasks will be scheduled
	BaseType_t ret_val = ESP_FAIL;
	BaseType_t relDeadlineMsTemp = 0;

	// Compute aperiodic utilization factor
	if (periodicUtil > 0)
		aperiodicUtil = 1 - periodicUtil;

	/* Compute fictitious aperiodic deadlines for Total Bandwidth Server
	   Di = max (Ai, Di-1) + Ci/ Us   */

	// printf("\n");
	for (int16_t i = 0; i < APP_APER_TASKS; i++)
	{
		// Checks to avoid divide by 0 and nonsensical values
		if (periodicUtil < 1)
		{
			// checks to ensure schedulability and plotting integrity
			configASSERT(apTask_arr[i].taskNumber != MAIN_TASK_NUM);
			configASSERT(apTask_arr[i].taskNumber != IDLE_TASK_NUM);
			configASSERT(apTask_arr[i].taskNumber != SCHED_TASK_NUM);
			exTCB_t *TCB_i = (exTCB_t *)malloc(sizeof(exTCB_t));

			apTask_arr[i].relDeadlineMs = (BaseType_t)(prvMaximum(apTask_arr[i].relArrivalMs, relDeadlineMsTemp) + (float)(apTask_arr[i].WcetMs / aperiodicUtil));
			relDeadlineMsTemp = apTask_arr[i].relDeadlineMs;

			// Initialize list TCB values
			TCB_i->periodTick = 100000; // Aperiodic modelled as infinite period
			TCB_i->phaseTick = 0;		// Irrelevant for aperiodic
			TCB_i->WcetUs = apTask_arr[i].WcetMs * 1000;
			TCB_i->relArrivalTick = pdMS_TO_TICKS(apTask_arr[i].relArrivalMs);
			TCB_i->relDeadlineTick = pdMS_TO_TICKS(apTask_arr[i].relDeadlineMs);
			TCB_i->absDeadlineTick = TCB_i->phaseTick + TCB_i->relDeadlineTick;

			TCB_i->taskCallbackFunction = apTask_arr[i].taskCallbackFunction;
			TCB_i->taskHandle = NULL;
			TCB_i->taskNumber = apTask_arr[i].taskNumber;
			TCB_i->phaseShifted = pdFALSE;
			TCB_i->isAperiodic = pdTRUE;

			// Add aperiodic TCBs to primary linked list.
			vListInitialiseItem(&TCB_i->xTCBListItem);
			listSET_LIST_ITEM_OWNER(&TCB_i->xTCBListItem, TCB_i);
			listSET_LIST_ITEM_VALUE(&TCB_i->xTCBListItem, TCB_i->absDeadlineTick);
			vListInsert(xTCBListMain, &TCB_i->xTCBListItem);
			numOfTasks++;
		}
		else
		{
			printf("[WARNING] Aperiodic tasks not schedulable by EDF. TBS allocated %d CPU utilization factor", aperiodicUtil);
		}
	}
	ret_val = ESP_OK;

	return ret_val;
}

// Fully initialize the TCB sturucture and insert it into the linked list
BaseType_t prvInit_periodic_tasks(pubTCB_t *pTask_arr)
{
	//  all  initialized periodic tasks will be scheduled
	BaseType_t ret_val = ESP_FAIL;
	for (int16_t i = 0; i < APP_PER_TASKS; i++)
	{
		exTCB_t *TCB_i = (exTCB_t *)malloc(sizeof(exTCB_t));

		// checks to ensure schedulability and plotting integrity
		configASSERT(pTask_arr[i].relDeadlineMs <= pTask_arr[i].periodMs);
		configASSERT(pTask_arr[i].taskNumber != MAIN_TASK_NUM);
		configASSERT(pTask_arr[i].taskNumber != IDLE_TASK_NUM);
		configASSERT(pTask_arr[i].taskNumber != SCHED_TASK_NUM);

		// Initialize list TCB values
		TCB_i->periodTick = pdMS_TO_TICKS(pTask_arr[i].periodMs);
		TCB_i->phaseTick = pdMS_TO_TICKS(pTask_arr[i].phaseMs);
		TCB_i->WcetUs = pTask_arr[i].WcetMs * 1000;
		TCB_i->relArrivalTick = 0;
		TCB_i->relDeadlineTick = pdMS_TO_TICKS(pTask_arr[i].relDeadlineMs);
		TCB_i->absDeadlineTick = TCB_i->phaseTick + TCB_i->relDeadlineTick;

		TCB_i->taskCallbackFunction = pTask_arr[i].taskCallbackFunction;
		TCB_i->taskHandle = NULL;
		TCB_i->taskNumber = pTask_arr[i].taskNumber;
		TCB_i->phaseShifted = pdFALSE;
		TCB_i->isAperiodic = pdFALSE;

		// Add periodic TCBs to primary linked list.
		vListInitialiseItem(&TCB_i->xTCBListItem);
		listSET_LIST_ITEM_OWNER(&TCB_i->xTCBListItem, TCB_i);
		listSET_LIST_ITEM_VALUE(&TCB_i->xTCBListItem, TCB_i->absDeadlineTick);
		vListInsert(xTCBListMain, &TCB_i->xTCBListItem);

		numOfTasks++;
		// Compute periodic utilization factor
		periodicUtil = periodicUtil + (float)(pTask_arr[i].WcetMs / (float)pTask_arr[i].periodMs);
	}
	ret_val = ESP_OK;

	return ret_val;
}

// Initialize TCB lists and assign initial priorities
BaseType_t EdfScheduleInit(pubTCB_t *pTask_arr, pubTCB_t *apTask_arr)
{

	int16_t ret_pval = ESP_FAIL;
	int16_t ret_apval = ESP_FAIL;

	exTCB_t *xTCBSort;
	ListItem_t *xListItem;
	const ListItem_t *xListEndMarker;
	BaseType_t priority = MAX_SYS_PRIO - 1;

	// Set main, idle task numbers to ease debugging of trace log
	mainTaskHandle = xTaskGetCurrentTaskHandle();
	vTaskSetTaskNumber(mainTaskHandle, MAIN_TASK_NUM);
	idleTaskHandle = xTaskGetIdleTaskHandle();
	vTaskSetTaskNumber(idleTaskHandle, IDLE_TASK_NUM);

	// sets max priority for the main task
	vTaskPrioritySet(mainTaskHandle, MAX_SYS_PRIO + 1);
	// vTaskDelay(pdMS_TO_TICKS(50));

	// create primary and temporary linked lists
	vListInitialise(xTCBListMain);
	vListInitialise(xTCBListTemp);

	// intitialize private periodic, aperiodic TCBs and add to primary linked list.
	ret_pval = prvInit_periodic_tasks(pTask_arr);
	ret_apval = prvInit_aperiodic_tasks(apTask_arr);

	// Initiliaze periodic task priorities according to implicitly sorted order
	printf("\n[DEBUG] EDF Utilization factors Up %f, Us %f", periodicUtil, aperiodicUtil);
	xListItem = listGET_HEAD_ENTRY(xTCBListMain);
	xListEndMarker = listGET_END_MARKER(xTCBListMain);
	while (xListItem != xListEndMarker)
	{
		xTCBSort = listGET_LIST_ITEM_OWNER(xListItem);
		xTCBSort->priority = priority;
		priority--;
		xListItem = listGET_NEXT(xListItem);
	}

	return (ret_pval && ret_apval);
}

// Create the periodic application tasks and the scheduler task
void EdfScheduleStart()
{
	printf("\n[DEBUG] EDF scheduler started");
	// application periodic tasks
	exTCB_t *xTCB;
	ListItem_t *xTCBListItem = listGET_HEAD_ENTRY(xTCBListMain);
	const ListItem_t *xTCBListEndMarker = listGET_END_MARKER(xTCBListMain);
	BaseType_t taskCreated = pdFALSE;

	if (0.95 < periodicUtil && periodicUtil <= 1)
	{
		printf("\n[WARNING] : High periodic task utilization factor %f. EDF unlikely to meet deadline due to overhead \n", periodicUtil);
	}

	else if (1 < periodicUtil)
	{
		printf("\n[ERROR] : Infeasible utilization factor %f. Deadline overflow impending \n", aperiodicUtil);
	}

	vTaskDelay(1);
	// Avoid print delays after this for accuracy of EDF time reference
	sysStartTick = xTaskGetTickCount();

	// Create registered periodic tasks from linked list
	while (xTCBListItem != xTCBListEndMarker)
	{

		xTCB = listGET_LIST_ITEM_OWNER(xTCBListItem);
		taskCreated = xTaskCreate(prvEdfTaskWrapper,
								  NULL,
								  4096,
								  (void *)xTCB,
								  xTCB->priority,
								  &(xTCB->taskHandle));

		if (taskCreated == pdPASS)
		{
			vTaskSetThreadLocalStoragePointer(xTCB->taskHandle, LOCAL_STORAGE_INDEX, xTCB);
		}
		else
		{
			printf("[ERROR] :Could not allocate memory\n");
			abort();
		}
		// Assign user configured numbers to periodic tasks
		vTaskSetTaskNumber(xTCB->taskHandle, xTCB->taskNumber);
		xTCBListItem = listGET_NEXT(xTCBListItem);
	}
	TasksCreated = pdTRUE;
}

// At system shutdown, delete all TCB in list
void EdfDeleteAllTasks()
{
	exTCB_t *xTCB;
	TaskHandle_t xHandle;
	ListItem_t *xTCBListItem = listGET_HEAD_ENTRY(xTCBListMain);
	const ListItem_t *xTCBListEndMarker = listGET_END_MARKER(xTCBListMain);
	while (xTCBListItem != xTCBListEndMarker)
	{
		xTCB = listGET_LIST_ITEM_OWNER(xTCBListItem);
		xTCBListItem = listGET_NEXT(xTCBListItem);
		xHandle = xTCB->taskHandle;
		prvDeleteTCBFromList(xTCB);
		if (xHandle != NULL)
		{
			vTaskDelete(xHandle);
		}
	}
}

/* Function invoked by kernel via traceTASK_SWITCHED_OUT() macro
   task deadline sorting and new priority assignment function */
void prvEdfUpdatePrioritiesTrace(void)
{
	if (TasksCreated == pdFALSE)
		return;

	exTCB_t *xTCB;
	ListItem_t *xListItem;
	ListItem_t *xListItemTmp;
	TaskHandle_t xHandle;

	const ListItem_t *xListEndMarker = listGET_END_MARKER(xTCBListMain);
	xListItem = listGET_HEAD_ENTRY(xTCBListMain);

	// update item vlaue (Abs Deadline) and add to temp list (sorted by kernel)
	while ((xListItem != xListEndMarker))
	{
		xTCB = listGET_LIST_ITEM_OWNER(xListItem);

		// Delete marked tasks, remove from list, and move on
		if ((xTCB->toDelete == pdTRUE))
		{
			xListItem = listGET_NEXT(xListItem);
			xHandle = xTCB->taskHandle;
			prvDeleteTCBFromList(xTCB);
			vTaskDelete(xHandle);
		}

		listSET_LIST_ITEM_VALUE(xListItem, xTCB->absDeadlineTick);
		xListItemTmp = xListItem;
		xListItem = listGET_NEXT(xListItem);
		// Remove from main list and insert into temp list to facilitate sorting
		uxListRemove(xListItem->pxPrevious);
		vListInsert(xTCBListTemp, xListItemTmp);
	}

	// swap sorted tempList with mainList.
	prvSwapLists(&xTCBListMain, &xTCBListTemp);

	const ListItem_t *xListEndAfterSwap = listGET_END_MARKER(xTCBListMain);
	xListItem = listGET_HEAD_ENTRY(xTCBListMain);
	BaseType_t priority = MAX_SYS_PRIO - 1;

	// Set priorities according to sorted order
	while (xListItem != xListEndAfterSwap)
	{
		// Update priorities
		xTCB = listGET_LIST_ITEM_OWNER(xListItem);
		xTCB->priority = priority;
		vTaskPrioritySet(xTCB->taskHandle, xTCB->priority);
		priority--;

		xListItem = listGET_NEXT(xListItem);
	}
}

// Swap two lists given their pointers
static void prvSwapLists(List_t **aTCBList, List_t **bTCBList)
{
	List_t *tmp = *aTCBList;
	*aTCBList = *bTCBList;
	*bTCBList = tmp;
}

// Delete item from list
static void prvDeleteTCBFromList(exTCB_t *xTCB)
{
	uxListRemove(&xTCB->xTCBListItem);
	vPortFree(xTCB);
}

// return the greater value among two floats
static float prvMaximum(float num1f, float num2f)
{
	float value = num2f;
	if (num1f >= num2f)
	{
		value = num1f;
	}
	return value;
}

// handles task instance setup and cleanup actions
static void prvEdfTaskWrapper(void *pvParameters)
{
	exTCB_t *TCB_pst = (exTCB_t *)pvParameters;

	// Tasks need to consider arrival time from sys startup
	TCB_pst->relArrivalTick = TCB_pst->relArrivalTick + sysStartTick;
	// Check deadline overflows require consistent definition of abs deadline
	TCB_pst->absDeadlineTick = TCB_pst->absDeadlineTick + sysStartTick;

	// Block aperiodic task till absolute arrival
	if (TCB_pst->isAperiodic == pdTRUE)
	{
		vTaskDelay(TCB_pst->relArrivalTick - xTaskGetTickCount());
	}

	// Block periodic task's first instance till absolute phase
	else if ((TCB_pst->phaseTick > 0) && (TCB_pst->phaseShifted == pdFALSE))
	{

		TCB_pst->phaseTick = TCB_pst->phaseTick + sysStartTick;
		xTaskDelayUntil(&TCB_pst->relArrivalTick, TCB_pst->phaseTick - TCB_pst->relArrivalTick);
		TCB_pst->phaseShifted = pdTRUE;
	}

	// record first non-blocked task instance for logging
	if (firstTaskUs == 0)
	{
		firstTaskUs = esp_timer_get_time();
	}
	// TODO BUG. Why first arrrival at tick 0, even if sys starts at tick1 ?

	while (1)
	{
		// Execute user specified task callback
		TCB_pst->taskCallbackFunction(TCB_pst->WcetUs);

		// Post task instance updates
		if (TCB_pst->isAperiodic)
		{
			// Mark to delete after first instace excecuted
			TCB_pst->toDelete = pdTRUE;
		}
		// Update next instance's deadline before task sent into blocked state
		TCB_pst->absDeadlineTick = TCB_pst->relArrivalTick + TCB_pst->relDeadlineTick + TCB_pst->periodTick;
		vTaskDelayUntil(&(TCB_pst->relArrivalTick), TCB_pst->periodTick);
	}
}

/* Function invoked by kernel via traceTASK_INCREMENT_TICK()
   to check for deadline overflows */
void prvEdfCheckDeadlineTrace()
{
	if (TasksCreated == pdFALSE)
		return;

	ListItem_t *xTCBListItem = listGET_HEAD_ENTRY(xTCBListMain);
	const ListItem_t *xListEndMarker = listGET_END_MARKER(xTCBListMain);

	exTCB_t *xTCB;

	while (xTCBListItem != xListEndMarker)
	{
		xTCB = listGET_LIST_ITEM_OWNER(xTCBListItem);
		xTCBListItem = listGET_NEXT(xTCBListItem);
		TickType_t curTick = xTaskGetTickCount();
		// For fresh deadline overflows, mark for deletion and suspend
		if ((((long int)xTCB->absDeadlineTick - (long int)curTick) < 0) && (xTCB->toDelete != pdTRUE))
		{
			taskLog[idx_gl][3] = esp_timer_get_time();
			xTCB->toDelete = pdTRUE;
			vTaskSuspend(xTCB->taskHandle);
		}
	}
}
