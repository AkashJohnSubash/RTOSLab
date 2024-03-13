#define APP_PER_NUM      2         // Number of application periodic tasks
#define APP_COMP_ITER    500000      // number of iteration corresponding to task computation time
#define APP_BASE_PRIO    2           // minimum prio of an aplpication task
#define DISPL_PRIO       (APP_BASE_PRIO + APP_PER_NUM+ 1)  // higher than all appl tasks
// higher number -> higher prio 
 
//-------------------------Task declaration-----------------------// 

// define application tasks as necessary
void app_pTask1 (void* pTsk_pst);
void app_pTask2 (void* pTsk_pst);
void app_pTask3 (void* pTsk_pst);


void displ_task(void* Unused);
//---------------------Global variable declaration-------------------//

// array of function pointers