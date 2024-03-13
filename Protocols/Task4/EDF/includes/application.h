#include "edf_scheduler.h"

//-------------------------------Defines--------------------------// 
#define APP_PER_TASKS      3
#define APP_APER_TASKS     0
#define NUM_SLICES         100
 
//-------------------------Function prototypes-----------------------// 
extern int idx_gl ;
extern int taskLog[LOG_NUM_EVNTS][LOG_NUM_FIELDS]; 

BaseType_t reg_periodic_tasks(pubTCB_t* pTask_arr); 
BaseType_t reg_aperiodic_tasks(pubTCB_t* apTask_arr); 

// define application tasks as necessary
void app_pTask1 (int WCETUs);
void app_pTask2 (int WCETUs);
void app_pTask3 (int WCETUs);
void app_pTask4 (int WCETUs);

void app_apTask1(int WCETUs);
void app_apTask2(int WCETUs);

void logToSerial(pubTCB_t* pTask_arr, pubTCB_t* apTask_arr);

//---------------------Global variable declaration-------------------//


// array of function pointers