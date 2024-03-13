import argparse
import re
import numpy as np
import matplotlib.pyplot as plt


NumOfPeriodicTasks = 0 #10 # Max num of periodic
NumOfAperiodicTasks = 0 #2 # Max num of aperiodic


IdleTask = 0
taskNumStart = 4
MainTask = 2

tickwidth = 1
offsetMs = 2


scheduleStart = 0
scheduleEnd = 0

periodicTaskNum = []
aperiodicTaskNum = []
periods = {}
phases = {}
aperiodicArrivals = {}
aperiodicDeadlines = {}

sysStartMs = 0
hyperPeriodMs = 1600


def generateVisu(outputFile):
    dataStart = False
    metaDataStart = False
    traceData = []
    global NumOfPeriodicTasks
    global NumOfAperiodicTasks

    with open(outputFile, "r") as file:
        f = file.readline()

        while(f != ''):
            f = file.readline()

            if "[INFO] Schedule Data Gathered. Printing data......" in f:
                dataStart = True
            elif "[INFO] Schedule Printed!!!" in f:
                dataStart = False
            elif "Task start time" in f:
                taskStartMs = int(re.findall("[0-9]+", f)[0]) / 1000
            elif "Metadata start" in f:
                metaDataStart = True

            elif "Metadata end" in f:
                metaDataStart = False

            elif "[INFO] Schedule printed!!!" in f:
                break
            if metaDataStart:
                metaData = re.findall("[0-9]+", f)
                if "Periodic" in f:
                    # generate periodic task references from metadata
                    periodicTaskNum.append(int(metaData[0]))
                    NumOfPeriodicTasks = NumOfPeriodicTasks + 1
                    periods[f"Periodic Task {metaData[0]}"] = int(metaData[4])
                    phases[f"Periodic Task {metaData[0]}"] = int(metaData[2])

                elif "Aperiodic" in f:
                    # generate Aperiodic task references from metadata
                    NumOfAperiodicTasks = NumOfAperiodicTasks + 1
                    aperiodicTaskNum.append(int(metaData[0]))
                    aperiodicArrivals[f"Aperiodic Task {metaData[0]}"] = int(metaData[1])
                    aperiodicDeadlines[f"Aperiodic Task {metaData[0]}"] = int(metaData[4])

            
            if dataStart:
                data = re.findall("[0-9]+", f)
                if (len(data) != 0):
                    traceData.append(list(map(int, data)))
                else :
                    continue
    
    traceData = np.array(traceData)
    generateStats(traceData)

    # process each task as column, first column are tick numbers
    data = []
    data2 = {}

    DetectedPeriodicTasks = []
    DetectedAperiodicTasks = []
    tasksToPlot = []

    earliestTaskStartTime = 0
    earliestTaskStartTimeRec = False

    # Postprocess stage 1
    for i in range(0, np.shape(traceData)[0]):
        if (traceData[i, :].any() == 0):
            continue
        
        taskNum = traceData[i, 1]

        # Detect idle task
        if (taskNum == IdleTask):
            taskName = "Idle Task"
            if taskName not in tasksToPlot:
                tasksToPlot.append(taskName)
        
        # Detect periodic tasks
        elif taskNum in periodicTaskNum:
            taskName = "Periodic Task " + str(taskNum)

            if taskNum not in DetectedPeriodicTasks:
                    DetectedPeriodicTasks.append(taskNum)
            if taskName not in tasksToPlot:
                    tasksToPlot.append(taskName)
        
        # Detect aperiodic tasks
        elif taskNum in aperiodicTaskNum:
            taskName = "Aperiodic Task " + str(taskNum)

            if taskNum not in DetectedAperiodicTasks:
                DetectedAperiodicTasks.append(taskNum)
            if taskName not in tasksToPlot:
                tasksToPlot.append(taskName)
        
        # Overlook main task for plot
        elif (taskNum == MainTask):
            pass
            # taskName = "Main Task"
            # if taskName not in tasksToPlot:
            #     tasksToPlot.append(taskName)
        
        obj = dict([("Task", taskName),
                    ("Start Time (ms)", traceData[i, 2] / 1000),
                    ("End Time (ms)", traceData[i, 3] / 1000),
                    ("Overflow Time (ms)", traceData[i, 4] / 1000)])

        data.append(obj)
        # Detect first valid task instance (Task offset)
        if taskName not in data2:
            data2[taskName] = []
            earliestTaskStartTime = taskStartMs - offsetMs
            earliestTaskStartTimeRec = True

        tup = (traceData[i, 2] / 1000, traceData[i, 3] / 1000 - traceData[i, 2] / 1000, traceData[i, 4] / 1000)
        data2[taskName].append(tup)
            
    # Postprocess stage 2
    # Shift all values to earliest task start time
    data3 = {}
    for key in data2:
        if key not in data3:
            data3[key] = []
        for i in data2[key]:
            if i[0] - earliestTaskStartTime < 0:
                continue
            tup = (i[0] - earliestTaskStartTime, i[1])
            data3[key].append(tup)

    heightStart = 10
    aperiodicColor = "steelblue"
    periodicColor = "lightcoral"
    schedColor = "brown"
    idleColor = "teal"
    height = 10
    ind = 0
    yTicksLabels = []
    fig, ax = plt.subplots()
    for key in data3.keys():
        if key == "Idle Task":
            color = idleColor
        elif "Aperiodic" in key:
            color = aperiodicColor
        elif "Periodic" in key:
            color = periodicColor
        else:
            color = schedColor
        ax.broken_barh(data3[key], (heightStart + height * ind, height), facecolors=color, label=key)

        if key in periods.keys():
            arrivalPoints = np.arange(phases[key], 4500, periods[key])
            deadlinePoints = arrivalPoints + periods[key]

            # plot arrival arrows 
            for i in arrivalPoints:
                if data2[key][-1][0] < i:
                    break
                if i == 0:
                    offset = 0
                else:
                    offset = tickwidth
                ax.annotate('', xy=(i  - offset, heightStart + height * ind), xytext=(0, 40), textcoords='offset points', arrowprops=dict(arrowstyle="<|-", color="black"))
            
            # plot deadline arrows
            for j in deadlinePoints:
                clr = 'black'
                # Plot overflowed deadlines in red (task ends at most 2 ticks after overflow detection)
                for k in data2[key]:
                    if (j <k[2]) and (k[2] < j + periods[key]):
                        clr = 'red'
                if j == 0:
                    offset = 0
                else:
                    offset = tickwidth
                ax.annotate('', xy=(j - offset, heightStart + height * ind), xytext=(0, 40), textcoords='offset points', arrowprops=dict(arrowstyle="-|>", color=clr))
                if data2[key][-2][0] < j:
                    break

        elif key in aperiodicArrivals.keys():
            aperiodicArrivalPoint = aperiodicArrivals[key]
        
            if aperiodicArrivalPoint == 0:
                offset = 0
            # mark aperiodic arrival arrow
            ax.annotate('', xy=(aperiodicArrivalPoint - offset, heightStart + height * ind), xytext=(0, 40), textcoords='offset points', arrowprops=dict(arrowstyle="<|-"))
            # mark aperiodic deadline arrow
            aperiodicDeadline = aperiodicDeadlines[key]
            ax.annotate('', xy=(aperiodicDeadline - offset, heightStart + height * ind), xytext=(0, 40), textcoords='offset points', arrowprops=dict(arrowstyle="-|>"))
        
        yTicksLabels.append(key)
        ind += 1

    
    ax.set_ylim(5, 15 + len(data2.keys()) * 10)
    # Todo dynamically detect x lim
    ax.set_xlim(0, hyperPeriodMs)
    ax.set_xlabel('Time (ms)')
    ax.set_yticks(np.arange(15, 15 + len(data2.keys()) * 10, 10))
    ax.set_yticklabels(yTicksLabels)
    ax.grid(True)

    plt.title("EDF Schedule")
    plt.legend()
    plt.show()


def generateStats(traceData):

    # generate Approximate utilization stats 
    # (peroidic, aperiodic, idle, main)
    data = np.zeros(4)
    for i in range(0, np.shape(traceData)[0]):

        if (traceData[i, 1:].any() == 0):
            continue
        else:
            if i == 1:
                scheduleStart = traceData[i, 2]
            
            # sum up execution time of periodic instances
            elif traceData[i, 1] in  periodicTaskNum:
                data[0] += traceData[i, 3] - traceData[i, 2]
            
            # sum up execution time of aperiodic tasks 
            elif traceData[i, 1] in  aperiodicTaskNum:
                data[1] += traceData[i, 3] - traceData[i, 2]
            
            # sum up execution time of idle task instances
            if traceData[i, 1] == IdleTask:
                data[2] += traceData[i, 3] - traceData[i, 2]
            
            # sum up execution time of main task instances
            if traceData[i, 1] == MainTask:
                data[3] += traceData[i, 3] - traceData[i, 2]

            if i == (np.shape(traceData)[0] - 1):
                scheduleEnd = traceData[i, 3]
    
    # Get total
    total = scheduleEnd - scheduleStart
    # Get periodic Contribution: indices 0:10
    periodicUtilization = np.sum(data[0]) / total
    # Get Aperiodic Count
    aperiodicUtilization = np.sum(data[1]) / total
    # get IDLE Utilization
    idleUtilization = np.sum(data[2]) / total
    mainUtilization = np.sum(data[3]) / total
    # get kernel utilization (total runtime - task execution time)
    kernelUtil = (total - np.sum(data)) / total

    # Print Stats
    print(f"[INFO] Time Measured: {total} us")
    print(f"[INFO] Periodic Utilization: {round(periodicUtilization*100, 3)}%")
    print(f"[INFO] Aperiodic Utilization: {round(aperiodicUtilization * 100, 3)}%")
    print(f"[INFO] IDLE Utilization: {round(idleUtilization * 100, 3)}%")
    print(f"[INFO] FreeRTOS Kernel Utilization, includes Trace Macros: {round(kernelUtil * 100, 3)} %")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("-f", "--file", required=True, help="Schedule File")
    args = vars(ap.parse_args())

    generateVisu(args["file"])