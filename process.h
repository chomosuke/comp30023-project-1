#ifndef PROCESS
#define PROCESS

#include "head.h"

typedef struct Process Process;
typedef struct Subprocess Subprocess;

Subprocess *newSubprocess(Process *parent, Time remainingTime, unsigned id, unsigned parentId);
void destorySubprocess(Subprocess *this);
Process *newProcess(Time arriveTime, ID id, Time exeTime, char parallelisable, int numCPU);
bool isFinished(Process *this);
void destroyProcess(Process *this);

struct Process { /* an OOP struct */
    Subprocess** children; /* non parallelisable process */
    int numChildren;       /* will still contain 1 subprocess */
    Time arriveTime;
    ID id;
    Time exeTime;
    bool parallelisable;
    Time startTime;
    Time finishTime;
}; /* Process will manage it's children */

struct Subprocess { /* an OOP struct */
    Process* parent;
    Time remainingTime;
    char id[16]; /* 10 for 2^32, 4 for up to 1024 CPU, 1 for . and 1 for \0 */
};

Process *newProcess(Time arriveTime, ID id, Time exeTime, char parallelisable, int numCPU) {

    Process *this = malloc(sizeof(Process));

    this->arriveTime = arriveTime;
    this->id = id;
    this->exeTime = exeTime;

    if (parallelisable == 'p') {
        this->parallelisable = true;
    } else if (parallelisable == 'n') {
        this->parallelisable = false;
    } else {
        printf("unrecognized parallelisable input.\n");
    }

    /* make children */
    this->numChildren = numCPU < this->exeTime ? numCPU : this->exeTime;
    this->children = malloc(this->numChildren * sizeof(Subprocess*));

    Time remainingTime = ceil(exeTime / this->numChildren) + 1;

    int i;
    for (i = 0; i < this->numChildren; i) {
        this->children[i] = newSubprocess(this, remainingTime, i, this->id);
    }

    return this;
}

bool isFinished(Process *this) {
    int i;
    for (i = 0; i < this->numChildren; i++) {
        if (this->children[i]->remainingTime != 0) {
            return false;
        }
    }
    return true;
}

void destroyProcess(Process *this) {
    int i;
    for (i = 0; i < this->numChildren; i++) {
        destorySubprocess(this->children[i]);
    }
    destroyProcess(this);
}

Subprocess *newSubprocess(Process *parent, Time remainingTime, unsigned id, unsigned parentId) {
    Subprocess *this = malloc(sizeof(Subprocess));
    this->parent = parent;
    if (parent->numChildren == 1) {
        sprintf(this->id, "%d", parentId);
    } else {
        sprintf(this->id, "%d.%d", parentId, id);
    }
    this->remainingTime = remainingTime;

    return this;
}

void destorySubprocess(Subprocess *this) {
    free(this);
}

#endif