#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef int bool;
#define true 1
#define false 0

typedef struct Process {
    unsigned arriveTime;
    unsigned id;
    unsigned exeTime;
    bool parallelisable;
} Process;

Process *readInputFile(const char *fileName, unsigned *size);

int main(int argc, char** argv) {

    /* read cmd arguments */
    char* fileName;
    bool c;
    int p, i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            i++;
            fileName = argv[i];
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            p = atoi(argv[i]);
        } else if (strcmp(argv[i], "-c") == 0) {
            c = true;
        } else {
            printf("unrecognized argument: %s\n", argv[i]);
        }
    }

    unsigned processesSize;
    Process* processes = readInputFile(fileName, &processesSize);

    for (i = 0; i < processesSize; i++) {
        printf("%u, %u, %u, %d\n", processes[i].arriveTime, processes[i].id, 
        processes[i].exeTime, processes[i].parallelisable);
    }

    free(processes);

    return 0;
}

Process *readInputFile(const char *fileName, unsigned *size) {
    /* read input file */
    FILE *file = fopen(fileName, "r");
    assert(file != NULL);

    unsigned i, processesSize = 4;
    Process currentProcess;
    Process* processes = malloc(processesSize * sizeof(Process));
    assert(processes != NULL);

    char p;
    for (i = 0;
        fscanf(file, "%u %u %u %c", 
        &currentProcess.arriveTime,
        &currentProcess.id, 
        &currentProcess.exeTime, &p) == 4
        ; i++) {
        if (p == 'p') {
            currentProcess.parallelisable = true;
        } else if (p == 'n') {
            currentProcess.parallelisable = false;
        } else {
            printf("unrecognized parallelisable input.\n");
        }

        if (i >= processesSize) {
            /* let's allocate some more */
            processesSize *= 2;
            processes = realloc(processes, processesSize * sizeof(Process));
        }
        processes[i] = currentProcess;
    }
    
    assert(fclose(file) == 0);

    *size = i;

    return processes;
}

/* tested using:
 gcc -Wall -o allocate allocate.c -std=c89
*/