#include <stdint.h>

typedef struct
{   
    int16_t     periodMs;                                       // periodic task period
    int16_t     phaseMs;                                        // periodic task phase
    // char  *     taskName;                                       // periodic task Name
    // int16_t     instanceNum;                                    // periodic task instance
    int16_t     priority;                                       // periodic task priority
    void  *     callback;
}pTaskParameters_t;

int16_t reg_periodic_tasks(pTaskParameters_t* pTask_arr); 
int16_t run_periodic_tasks(pTaskParameters_t* pTask_arr);