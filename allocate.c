#include "head.h"
#include "process.h"
#include "events.h"
#include "cpu.h"
#include "processesWaiting.h"

Process **readProcesses(const char *fileName, unsigned *size);
int getMaxNumChildren(Process* process, int numCPU);
Events *allocateNormal(Process** processes, unsigned processesSize, CPU** cpus, int numCPU);
Events *allocateChallenge(Process** processes, unsigned processesSize, CPU** cpus, int numCPU);
Events *runProcesses(Process **processes, unsigned processesSize, int numCPU, Events* (*allocate)(Process**, unsigned, CPU**, int));
void sortProcesses(Process** processes, unsigned processesSize);
bool processesInOrder(Process* a, Process* b);
void printResults(Events *events, Process **processes, unsigned processesSize);
double mRound(double d);

int main(int argc, char **argv) {

    /* read cmd arguments */
    /*char* fileName = "E:/Study/TODO/comp30023-2021-project-1/testcases/task7/test_chal_p6_n_equal.txt";
    int numCPU = 6, i; */
    char* fileName;
    int numCPU, i; 
    bool challenge = false;
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
    Process** processes = readProcesses(fileName, &processesSize);

    /* go through processes one by one and allocate their child to CPUs and collect their events */
    Events* (*allocate)(Process**, unsigned, CPU**, int) = challenge ? allocateChallenge : allocateNormal;
    Events *events = runProcesses(processes, processesSize, numCPU, allocate);

    /* sort events */
    printResults(events, processes, processesSize);

    for (i = 0; i < processesSize; i++) {
        destroyProcess(processes[i]);
    }
    
    free(processes);

    destroyEvents(events);

    return 0;
}

Process **readProcesses(const char *fileName, unsigned *size) {
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

        processes[i] = newProcess(arriveTime, id, exeTime, p);

    }

    assert(fclose(file) == 0);

    *size = i;

    return processes;
}

int getMaxNumChildren(Process* process, int numCPU) {
    if (!process->parallelisable) {
        return 1;
    }
    if (numCPU == 2) { /* 2 cpu will always split the process */
        return 2;
    }
    return numCPU < process->exeTime ? numCPU : process->exeTime;
}

Events *allocateNormal(Process** processes, unsigned processesSize, CPU** cpus, int numCPU) {

    Events *events = newEvents();

    bool *allocated = malloc(numCPU * sizeof(bool));
    assert(allocated != NULL);
    Time lastTime = 0;

    unsigned i;
    /* allocate processes to cpu one by one */ 
    for (i = 0; i < processesSize; i++) {

        makeChildren(processes[i], getMaxNumChildren(processes[i], numCPU));

        Time currentTime = processes[i]->arriveTime;

        int j;

        /* elapse time for all cpu and mark them unallocated */
        for (j = 0; j < numCPU; j++) {
            allocated[j] = false;
            Events* newEvents = elapseTime(cpus[j], lastTime, currentTime);
            concatAndDestroyOther(events, newEvents);
        }

        for (j = 0; j < processes[i]->numChildren; j++) {

            /* allocate the subprocess */

            /* find cpu with shortest remaining time that's not allocated yet */
            int k;
            int shortestCpu = 0;
            while (allocated[shortestCpu]) {
                shortestCpu++; /* first unallocated cpu */
            }

            for (k = 1; k < numCPU; k++) {
                if (!allocated[k]
                 && cpus[k]->remainingQueueTime < cpus[shortestCpu]->remainingQueueTime) {
                    shortestCpu = k;
                }
            }

            addToQueue(cpus[shortestCpu], processes[i]->children[j]);

            allocated[shortestCpu] = true;
        }

        lastTime = currentTime;
    }

    /* run all the remaining processes */
    for (i = 0; i < numCPU; i++) {
        Events *newEvents = finishAllProcesses(cpus[i], lastTime);
        concatAndDestroyOther(events, newEvents);
    }

    free(allocated);

    return events;
}

Events *allocateChallenge(Process** processes, unsigned processesSize, CPU** cpus, int numCPU) {
    Events *events = newEvents();

    Time time = 0;
    ProcessesWaiting *waitingRoom = newProcessesWaiting();
    unsigned i = 0;
    while (i < processesSize || !noProcess(waitingRoom)) {
        while (i < processesSize && time == processes[i]->arriveTime) {
            /* add to waiting room */
            addProcess(waitingRoom, processes[i]);
            i++;
        }

        int j;
        for (j = 0; j < numCPU && !noProcess(waitingRoom); j++) {
            if (cpus[j]->remainingQueueTime == 0) {
                /* allocate the biggest process to it */
                Process *process = popBiggest(waitingRoom);
                makeChildren(process, 1);
                addToQueue(cpus[j], process->children[0]);
            }
        }

        time++;
        for (j = 0; j < numCPU; j++) {
            concatAndDestroyOther(events, elapseTime(cpus[j], time - 1, time));
        }
    }

    
    /* run all the remaining processes */
    for (i = 0; i < numCPU; i++) {
        Events *newEvents = finishAllProcesses(cpus[i], time);
        concatAndDestroyOther(events, newEvents);
    }

    return events;
}

Events *runProcesses(Process** processes, unsigned processesSize, int numCPU, Events* (*allocate)(Process**, unsigned, CPU**, int)) {

    sortProcesses(processes, processesSize);

    /* initialize some CPU */
    CPU** cpus = malloc(numCPU * sizeof(CPU*));
    assert(cpus != NULL);
    unsigned i;
    for (i = 0; i < numCPU; i++) {
        cpus[i] = newCPU(i);
    }

    Events *events = allocate(processes, processesSize, cpus, numCPU);

    for (i = 0; i < numCPU; i++) {
        destroyCPU(cpus[i]);
    }
    free(cpus);

    return events;
}

/* insertion sort as processes could be almost sorted */
void sortProcesses(Process** processes, unsigned processesSize) {
    int i;
    for (i = 1; i < processesSize; i++) {
        int j;
        for (j = i - 1; j >= 0 && !processesInOrder(processes[j], processes[j + 1]); j--) {
            /* if not in order, swap */
            Process* temp = processes[j];
            processes[j] = processes[j + 1];
            processes[j + 1] = temp;
        }
    }
}

bool processesInOrder(Process* a, Process* b) {
    if (a->arriveTime != b->arriveTime) {
        return a->arriveTime < b->arriveTime;
    }
    if (a->exeTime != b->exeTime) {
        return a->exeTime < b->exeTime;
    }
    return a->id < b->id;
}

void printResults(Events *events, Process **processes, unsigned processesSize) {
    sortEvents(events);

    unsigned p = 0, e = 0;
    unsigned procRemaining = 0;
    unsigned i;
    for (i = 0; i < events->length; i++) {
        Event *event = events->array[i];

        /* update procRemaining */
        while (p < processesSize && processes[p]->arriveTime < event->currentTime) {
            procRemaining++;
            p++;
        }
        while (e < events->length
            && events->array[e]->currentTime <= event->currentTime) {
            if (events->array[e]->type == FINISHED) {
                procRemaining--;
            }
            e++;
        }


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

    /* stats */
    Time totalTurnAround = 0;
    double maxOverhead = 0, sumOverhead = 0;
    for (i = 0; i < processesSize; i++) {
        Time turnAround = processes[i]->finishTime - processes[i]->arriveTime;
        totalTurnAround += turnAround;
        double overhead = (double)turnAround / processes[i]->exeTime;
        maxOverhead = overhead > maxOverhead ? overhead : maxOverhead;
        sumOverhead += overhead;
    }
    Time averageTurnAround = ceil((double)totalTurnAround / processesSize);
    double averageOverhead = mRound(sumOverhead / processesSize * 100) / 100;
    maxOverhead = mRound(maxOverhead * 100) / 100;
    Time makeSpan = events->array[events->length - 1]->currentTime;
    printf("Turnaround time %u\nTime overhead %g %g\nMakespan %u\n", 
        averageTurnAround, maxOverhead, averageOverhead, makeSpan);
}

double mRound(double d) {
    return (d >= floor(d) + 0.5) ? ceil(d) : floor(d);
}

/* tested using:
gcc -Wall -o allocate allocate.c events.c process.c cpu.c processesWaiting.c -std=c89 -lm
*/