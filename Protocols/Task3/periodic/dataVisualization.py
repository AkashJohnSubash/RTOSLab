import altair as at
import re
import pandas as pd
import argparse
at.renderers.enable('mimetype')


def generateVisu(outputFile):
    dataStart = False

    tickList = []
    taskList = []

    with open(outputFile, "r") as file:
        f = file.readline()

        while(f != ''):
            f = file.readline()
            # print(f"DEBUG line {f}")
            if "[INFO] Schedule Data Gathered. Printing data......" in f:
                dataStart = True
                # print(f"DEBUG START Found")
            elif "[INFO] Schedule Printed!!!" in f:
                dataStart = False

            if dataStart:
                data = re.findall("[0-9]+", f)
                if len(data) != 2:
                    continue
                tickList.append(int(data[0]))
                taskList.append(int(data[1]))
    # print(f"Debug TaskList{taskList}, TickList{tickList}")
    # preprocess arrays
    multipleTicksIndices = []
    for i in range(1, len(taskList)):

        if taskList[i] != 0 and taskList[i-1] == 0:
            for ind in multipleTicksIndices:
                taskList[ind] = taskList[i]
            multipleTicksIndices = []
        elif taskList[i] == 0:
            if i == (len(taskList)-1):
                taskList = taskList[:multipleTicksIndices[0]]
                tickList = tickList[:multipleTicksIndices[0]]
                break 
            multipleTicksIndices.append(i)

    if taskList[0] == 0:
        taskList[0] = taskList[1]
    data = []

    for i in range(len(taskList)):
        task = "Task " + str(taskList[i] - 3)
        if taskList[i] == 2:
            task = "Main Task"
        elif taskList[i] == 3:
            task = "Idle Task"
        if (i > 0) and (taskList[i] == taskList[i-1]):
            prevTaskStart = data[-1]["Tick Start"]
            obj = dict([
                ("Task", task),
                ("Tick Start", prevTaskStart),
                ("Tick End", tickList[i])
            ])
            data.pop()
        else:
            obj = dict([
                ("Task", task),
                ("Tick Start", tickList[i] - 1),
                ("Tick End", tickList[i])
            ])
        
        data.append(obj)
                                # remove last entry (idle task end time incorrect)
    chart = at.Chart(pd.DataFrame(data[:-2])).mark_bar().encode(
        x = "Tick Start",
        x2 = "Tick End",
        y = "Task",
        color=at.Color("Task", legend=at.Legend(title="Task/Setup Code"))
    ).properties(
        title = "FreeRTOS Schedule for {} Tasks".format(max(taskList) - 3)
    )

    chart.save("visu2.html")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("-f", "--file", required=True, help="Schedule File")
    args = vars(ap.parse_args())

    generateVisu(args["file"])








