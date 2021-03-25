#include "process.h"

Subprocess *newSubprocess(Process *parent, Time remainingTime, ID id);
void destorySubprocess(Subprocess *this);

Subprocess *newSubprocess(Process *parent, Time remainingTime, ID id) {
    Subprocess *this = malloc(sizeof(Subprocess));
    this->parent = parent;
    this->id = id;
    this->remainingTime = remainingTime;

    return this;
}

void destorySubprocess(Subprocess *this) {
    free(this);
}

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
    if (this->parallelisable) {
        if (numCPU == 2) { /* 2 cpu will always split the process */
            this->numChildren = 2;
        } else {
            this->numChildren = numCPU < this->exeTime ? numCPU : this->exeTime;
        }
    } else {
        this->numChildren = 1;
    }
    this->children = malloc(this->numChildren * sizeof(Subprocess*));

    Time remainingTime = ceil(exeTime / this->numChildren) + 1;

    int i;
    for (i = 0; i < this->numChildren; i++) {
        this->children[i] = newSubprocess(this, remainingTime, i);
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
    free(this);
}