#ifndef PROCESS
#define PROCESS

#include "head.h"

typedef struct Process Process;
typedef struct Subprocess Subprocess;

Process *newProcess(Time arriveTime, ID id, Time exeTime, char parallelisable, int numCPU);
bool isFinished(Process *this);
void recordSubprocessFinished(Process* this, Time currentTime);
void destroyProcess(Process *this);

struct Process { /* an OOP struct */
    Subprocess** children; /* non parallelisable process */
    int numChildren;       /* will still contain 1 subprocess */
    Time arriveTime;
    ID id;
    Time exeTime;
    bool parallelisable;
    bool finishRecorded;
    Time finishTime;
}; /* Process will manage it's children */

struct Subprocess { /* an OOP struct */
    Process* parent;
    Time remainingTime;
    ID id;
};

#endif