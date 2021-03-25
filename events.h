#ifndef EVENTS
#define EVENTS

#include "head.h"

typedef struct Event Event;
typedef struct Events Events;

/* returned by elapseTimeAndAddToQueue() and interpreted by main */
struct Events { /* an OOP struct */
    Event** array;
    unsigned length;
    unsigned size;
};

struct Event { /* will only exist in Events */
    Time currentTime;
    char type[9]; /* FINISHED or RUNNING */
    ID pid;

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

void addRunning(Events *this, Time currentTime, ID pid, Time remainingTime, int cpu) {
    Event *event = malloc(sizeof(Event));
    event->currentTime = currentTime;
    strcpy(event->type, "RUNNING");
    event->pid = pid;
    event->remainingTime = remainingTime;
    event->cpu = cpu;
    addEvent(this, event);
}

void addFinished(Events *this, Time currentTime, ID pid, unsigned procRemaining) {
    Event *event = malloc(sizeof(Event));
    event->currentTime = currentTime;
    strcpy(event->type, "FINISHED");
    event->pid = pid;
    event->procRemaining = procRemaining;
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