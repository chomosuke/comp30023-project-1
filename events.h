#ifndef EVENTS
#define EVENTS

#include "head.h"
#include "process.h"

typedef struct Event Event;
typedef struct Events Events;

Events *newEvents();
void addRunning(Events *this, Time currentTime, Subprocess* subprocess, int cpu);
void addFinished(Events *this, Time currentTime, Process* process);
void concatAndDestroyOther(Events *this, Events *other);
void sortEvents(Events *this);
void destroyEvents(Events *this);

/* returned by elapseTimeAndAddToQueue() and interpreted by main */
struct Events { /* an OOP struct */
    Event** array;
    unsigned length;
    unsigned size;
};

struct Event { /* will only exist in Events */
    Time currentTime;
    Type type; /* FINISHED or RUNNING */
    char pid[IDLENGTH];
    int cpu;

    /* for RUNNING */
    Time remainingTime;
};

#endif