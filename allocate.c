#include "head.h"
#include "process.h"
#include "events.h"
#include "cpu.h"

Process **readProcesses(const char *fileName, unsigned *size, int numCPU);
Events *runProcesses(Process **processes, int numCPU);
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
    Events *events = runProcesses(processes, numCPU);

    /* sort events */
    printResults(events, processes, processesSize);

    for (i = 0; i < processesSize; i++) {
        destroyProcess(processes[i]);
    }
    free(processes);

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

Events *runProcesses(Process** processes, int numCPU) {
    /* initialize some CPU */
    CPU** cpus = malloc(numCPU * sizeof(CPU*));
    int i;
    for (i = 0; i < numCPU; i++) {
        cpus[i] = newCPU();
    }

    /*  */
    

    for (i = 0; i < numCPU; i++) {
        destroyCPU(cpus[i]);
    }
    free(cpus);
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
            printf("%u,RUNNING,pid=%u,remaining_time=%u,cpu=%d",
                   event->currentTime, event->pid, event->remainingTime, event->cpu);
        } else if (event->type == FINISHED) {
            printf("%u,FINISHED,pid=%u,proc_remaining=%u",
                   event->currentTime, event->pid, procRemaining);
        } else {
            printf("Event type error");
        }
    }
}

/* tested using:
 gcc -Wall -o allocate allocate.c -std=c89
*/