#include "head.h"
#include "process.h"
#include "events.h"
#include "cpu.h"

Process **readProcesses(const char *fileName, unsigned *size, int numCPU);
Events *runProcesses(Process **processes, unsigned processesSize, int numCPU);
void printResults(Events *events, Process **processes, unsigned processesSize);

int main(int argc, char **argv) {

    /* read cmd arguments */
    char* fileName;
    bool challenge;
    int numCPU, i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            i++;
            fileName = argv[i];
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            numCPU = atoi(argv[i]);
        } else if (strcmp(argv[i], "-c") == 0) {
            challenge = true;
        } else {
            printf("unrecognized argument: %s\n", argv[i]);
        }
    }

    unsigned processesSize;
    Process** processes = readProcesses(fileName, &processesSize, numCPU);

    /* go through processes one by one and allocate their child to CPUs and collect their events */
    Events *events = runProcesses(processes, processesSize, numCPU);


    /* sort events */
    printResults(events, processes, processesSize);



    for (i = 0; i < processesSize; i++) {
        destroyProcess(processes[i]);
    }
    
    free(processes);

    destroyEvents(events);

    return 0;
}

Process **readProcesses(const char *fileName, unsigned *size, int numCPU) {
    /* read input file */
    FILE *file = fopen(fileName, "r");
    assert(file != NULL);

    unsigned i, processesSize = 4;
    Process** processes = malloc(processesSize * sizeof(Process*));
    assert(processes != NULL);

    char p;
    Time arriveTime, exeTime;
    ID id;
    for (i = 0;
        fscanf(file, "%u %u %u %c", &arriveTime, &id, &exeTime, &p) == 4;
        i++) {

        if (i >= processesSize) {
            /* let's allocate some more */
            processesSize *= 2;
            processes = realloc(processes, processesSize * sizeof(Process*));
            assert(processes != NULL);
        }

        processes[i] = newProcess(arriveTime, id, exeTime, p, numCPU);

    }

    assert(fclose(file) == 0);

    *size = i;

    return processes;
}

Events *runProcesses(Process** processes, unsigned processesSize, int numCPU) {

    Events *events = newEvents();

    /* initialize some CPU */
    CPU** cpus = malloc(numCPU * sizeof(CPU*));
    unsigned i;
    for (i = 0; i < numCPU; i++) {
        cpus[i] = newCPU(i);
    }

    bool *allocated = malloc(numCPU * sizeof(bool));

    Time *lastAllocated = malloc(numCPU * sizeof(Time));
    for (i = 0; i < numCPU; i++) {
        lastAllocated[i] = 0;
    }

    /* allocate processes to cpu one by one */ 
    for (i = 0; i < processesSize; i++) {
        int j;

        /* all cpu unallocated at the start */
        for (j = 0; j < numCPU; j++) {
            allocated[j] = false;
        }

        for (j = 0; j < processes[i]->numChildren; j++) {

            /* allocate the subprocess */

            /* find cpu with shortest remaining time that's not allocated yet */
            int k;
            int shortestCpu = 0;
            for (k = 1; k < numCPU; k++) {
                if (!allocated[k]
                 && cpus[k]->remainingQueueTime < cpus[shortestCpu]->remainingQueueTime) {
                    shortestCpu = k;
                }
            }

            Events *newEvents = 
                elapseTimeAndAddToQueue(cpus[shortestCpu], lastAllocated[shortestCpu], processes[i]->children[j], processes[i]->arriveTime);

            concatAndDestroyOther(events, newEvents);

            allocated[shortestCpu] = true;
            lastAllocated[shortestCpu] = processes[i]->arriveTime;
        }
    }


    /* run all the remaining processes */
    for (i = 0; i < numCPU; i++) {
        Events *newEvents = finishAllProcesses(cpus[i], lastAllocated[i]);
        concatAndDestroyOther(events, newEvents);
    }

    for (i = 0; i < numCPU; i++) {
        destroyCPU(cpus[i]);
    }
    free(cpus);

    free(allocated);
    free(lastAllocated);

    return events;
}

void printResults(Events *events, Process **processes, unsigned processesSize) {
    sortEvents(events);

    unsigned j = 0;
    unsigned procRemaining = 0;

    Time totalTurnAround = 0;

    unsigned i;
    for (i = 0; i < events->length; i++) {
        Event *event = events->array[i];
        if (event->type == RUNNING) {
            printf("%u,RUNNING,pid=%s,remaining_time=%u,cpu=%d\n",
                   event->currentTime, event->pid, event->remainingTime, event->cpu);
        } else if (event->type == FINISHED) {
            printf("%u,FINISHED,pid=%s,proc_remaining=%u\n",
                   event->currentTime, event->pid, procRemaining);
        } else {
            printf("Event type error");
        }
    }
}

/* tested using:
 gcc -Wall -o allocate allocate.c events.c process.c cpu.c -std=c89
*/