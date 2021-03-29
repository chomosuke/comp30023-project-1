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

Process *newProcess(Time arriveTime, ID id, Time exeTime, char parallelisable) {

    Process *this = malloc(sizeof(Process));
    assert(this != NULL);

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

    this->finishRecorded = false;

    return this;
}

void makeChildren(Process *this, int numChildren) {
    this->numChildren = numChildren;
    this->children = malloc(this->numChildren * sizeof(Subprocess*));

    Time remainingTime;
    if (this->numChildren == 1) {
        remainingTime = this->exeTime;
    }
    else {
        remainingTime = ceil((float)this->exeTime / this->numChildren) + 1;
    }

    int i;
    for (i = 0; i < this->numChildren; i++) {
        this->children[i] = newSubprocess(this, remainingTime, i);
    }
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

void recordSubprocessFinished(Process *this, Time currentTime, int cpuId) {
    if (this->finishRecorded) {
        if (currentTime > this->finishTime) {
            /* this is a later time */
            this->finishTime = currentTime;
            this->finishCpuId = cpuId;
        } else if (currentTime == this->finishTime) {
            if (cpuId < this->finishCpuId) {
                this->finishCpuId = cpuId;
            }
        }
    }
    else {
        this->finishTime = currentTime;
        this->finishCpuId = cpuId;
        this->finishRecorded = true;
    }
}

void destroyProcess(Process *this) {
    int i;
    for (i = 0; i < this->numChildren; i++) {
        destorySubprocess(this->children[i]);
    }
    free(this);
}