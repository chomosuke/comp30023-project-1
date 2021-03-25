#ifndef EVENTS
#define EVENTS

#include "head.h"

typedef struct Event Event;
typedef struct Events Events;

Events *newEvents();
void addEvent(Events *this, Event* event);
void addRunning(Events *this, Time currentTime, Subprocess* subprocess, int cpu);
void addFinished(Events *this, Time currentTime, Process* process);
void concatAndDestroyOther(Events *this, Events *other);
void destroyEvents(Events *this);

/* returned by elapseTimeAndAddToQueue() and interpreted by main */
struct Events { /* an OOP struct */
    Event** array;
    unsigned length;
    unsigned size;
};

struct Event { /* will only exist in Events */
    Time currentTime;
    char type[9]; /* FINISHED or RUNNING */
    char pid[IDLENGTH];

    /* for RUNNING */
    Time remainingTime;
    int cpu;

    /* for FINISHED */
    unsigned procRemaining;
};

Events *newEvents() {
    Events *this = malloc(sizeof(Events));
    this->length = 0;
    this->size = 2;
    this->array = malloc(this->size * sizeof(Event*));

    return this;
}

void addEvent(Events *this, Event* event) {
    this->length += 1;
    if (this->length > this->size) {
        this->size *= 2;
        this->array = realloc(this->array, this->size * sizeof(Event*));
        assert(this->array != NULL);
    }
    this->array[this->length - 1] = event;
}

void addRunning(Events *this, Time currentTime, Subprocess* subprocess, int cpu) {
    Event *event = malloc(sizeof(Event));
    event->currentTime = currentTime;
    strcpy(event->type, "RUNNING");
    strcpy(event->pid, subprocess->id);
    event->remainingTime = subprocess->remainingTime;
    event->cpu = cpu;
    addEvent(this, event);
}

void addFinished(Events *this, Time currentTime, Process* process) {
    Event *event = malloc(sizeof(Event));
    event->currentTime = currentTime;
    strcpy(event->type, "FINISHED");
    sprintf(event->pid, "%u", process->id);
    /* procRemaining will be calculated by main when it's sorting events */
    addEvent(this, event);
}

void concatAndDestroyOther(Events *this, Events *other) {
    int i;
    for (i = 0; i < other->length; i++) {
        addEvent(this, other->array[i]);
    }
    free(other->array);
    free(other);
}

void destroyEvents(Events *this) {
    /* free array and things in array */
    int i;
    for (i = 0; i < this->length; i++) {
        free(this->array[i]);
    }
    free(this->array);
    free(this);
}

#endif