#include "head.h"
#include "process.h"
#include "events.h"
#include "cpu.h"

Process **readProcesses(const char *fileName, unsigned *size, int numCPU);
Events *runProcesses(Process **processes, unsigned processesSize, int numCPU);
void sortProcesses(Process** processes, unsigned processesSize);
bool processesInOrder(Process* a, Process* b);
void printResults(Events *events, Process **processes, unsigned processesSize);
double mRound(double d);

int main(int argc, char **argv) {

    /* read cmd arguments */
    /* char* fileName = "E:/Study/TODO/comp30023-2021-project-1/testcases/task4/input/test_p4_n_2.txt";
    int numCPU = 4, i; */
    char* fileName;
    int numCPU = 4, i;
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

    sortProcesses(processes, processesSize);

    Events *events = newEvents();

    /* initialize some CPU */
    CPU** cpus = malloc(numCPU * sizeof(CPU*));
    unsigned i;
    for (i = 0; i < numCPU; i++) {
        cpus[i] = newCPU(i);
    }

    bool *allocated = malloc(numCPU * sizeof(bool));
    assert(allocated != NULL);
    Time lastTime = 0;

    /* allocate processes to cpu one by one */ 
    for (i = 0; i < processesSize; i++) {

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

    for (i = 0; i < numCPU; i++) {
        destroyCPU(cpus[i]);
    }
    free(cpus);

    free(allocated);

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
gcc -Wall -o allocate allocate.c events.c process.c cpu.c -std=c89 -lm
*/