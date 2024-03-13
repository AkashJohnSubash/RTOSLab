# Introduction 
This module focuses on using the user space EDF scheduler for aperiodic and periodic tasks with FreeRTOS on the ESP32
(Task 4 option 1)

# Setup
Configuration for the Single core scheduler once the ESP idf has been setup

*Step                            : CLI command*
1. menu config                   : ```idf.py menuconfig```
                                 ->```Component config```  
                                 ->```FreeRTOS``  
                                 ->```Kernel```  
                                 ``` Run FreeRTOS on first core```  
                                 ``` use trace facility```     
2. To use the Trace macros defined for scheduling and logging 
  replace ```\components\freertos\esp_additions\include\\FreeRTOSConfig.h```  with   ```\includes\FreeRTOSConfig.h```  
                                         
3. build and flash            : ```idf.py <serialPortName> flash```      
4. write std out to log file  : ```idf.py <serialPortName> monitor > log.txt```
5. visualize schedule         : ```python EDFDataVisualizationESPTimer.py -f log.txt```  **

*prerequisite python3 packages : matplotlib, numpy