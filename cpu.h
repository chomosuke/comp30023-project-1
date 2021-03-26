#ifndef CPUH
#define CPUH

#include "head.h"
#include "events.h"
#include "process.h"

typedef struct CPU CPU;
typedef struct SubprocessNode SubprocessNode;

CPU *newCPU(int id);
Events *finishAllProcesses(CPU *this, Time time);
Events *elapseTime(CPU *this, Time lastTime, Time currentTime);
void addToQueue(CPU *this, Subprocess *toAdd);
void destroyCPU(CPU *this);

struct CPU { /* an OOP struct */
    SubprocessNode *head;
    Time remainingQueueTime;
    int id;
    bool processRunning;
};

#endif