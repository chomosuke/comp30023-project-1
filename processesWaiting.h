#ifndef PROCESSESWAITING
#define PROCESSESWAITING

#include "head.h"
#include "process.h"

typedef struct ProcessesWaiting ProcessesWaiting;
typedef struct ProcessNode ProcessNode;

ProcessesWaiting *newProcessesWaiting();
Process *popBiggest(ProcessesWaiting *this);
Process *popSmallest(ProcessesWaiting *this);
void addProcess(ProcessesWaiting *this, Process *process);
bool noProcess(ProcessesWaiting *this);
void destoryProcess(ProcessesWaiting *this);

struct ProcessesWaiting {
    ProcessNode *biggest;
    ProcessNode *smallest;
};

struct ProcessNode {
    ProcessNode *bigger;
    Process *process;
    ProcessNode *smaller;
};

#endif