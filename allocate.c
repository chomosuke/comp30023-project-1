#include "head.h"
#include "process.h"
#include "events.h"

Process **readProcesses(const char *fileName, unsigned *size, int numCPU);

int main(int argc, char** argv) {

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
    Process* processes = readProcesses(fileName, &processesSize, numCPU);

    // for (i = 0; i < processesSize; i++) {
    //     printf("%u, %u, %u, %d\n", processes[i].arriveTime, processes[i].id,
    //     processes[i].exeTime, processes[i].parallelisable);
    // }

    int sec = 0;
    for (i = 0; i < processesSize; i++) {
        // if (processes[i].parallelisable)
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

/* tested using:
 gcc -Wall -o allocate allocate.c -std=c89
*/