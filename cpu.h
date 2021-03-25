#ifndef CPUH
#define CPUH

#include "head.h"
#include "events.h"
#include "process.h"

typedef struct CPU CPU;
typedef struct SubprocessNode SubprocessNode;

CPU *newCPU();
Events *finishAllProcesses(CPU *this, Time time);
Events *elapseTimeAndAddToQueue(CPU *this, Time lastTime, Subprocess *toAdd, Time addTime);
void destroyCPU(CPU *this);

struct CPU { /* an OOP struct */
    SubprocessNode *head;
    Time remainingQueueTime;
    int id;
};

struct SubprocessNode { /* this will be managed by CPU */
    Subprocess *subprocess; /* cpu only run subprocesses */
    SubprocessNode *next;
};


#endif