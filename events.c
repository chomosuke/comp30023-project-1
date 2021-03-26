#include "events.h"

void addEvent(Events *this, Event* event);
void mergeSortEvents(Events *this, unsigned start, unsigned end);
bool eventsInOrder(Event *a, Event *b);

Events *newEvents() {
    Events *this = malloc(sizeof(Events));
    assert(this != NULL);
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
    assert(event != NULL);
    event->currentTime = currentTime;
    event->type = RUNNING;
    if (subprocess->parent->parallelisable) {
        sprintf(event->pid, "%d.%d", subprocess->parent->id, subprocess->id);
    } else {
        sprintf(event->pid, "%d", subprocess->parent->id);
    }
    event->remainingTime = subprocess->remainingTime;
    event->cpu = cpu;
    addEvent(this, event);
}

void addFinished(Events *this, Time currentTime, Process* process) {
    Event *event = malloc(sizeof(Event));
    event->currentTime = process->finishTime;
    event->type = FINISHED;
    sprintf(event->pid, "%u", process->id);
    /* procRemaining will be calculated by main when it's sorting events */
    addEvent(this, event);
}

void sortEvents(Events *this) {
    mergeSortEvents(this, 0, this->length);
}

void mergeSortEvents(Events *this, unsigned start, unsigned end) {
    if (end - start <= 1) {
        return;
    }
    
    /* split */
    unsigned mid = (start + end) / 2;
    mergeSortEvents(this, start, mid);
    mergeSortEvents(this, mid, end);

    long long resultSize = (end - start) * sizeof(Event*);
    Event** result = malloc(resultSize);
    unsigned i, s = 0, m = 0;
    for (i = 0; i < end - start; i++) {
        if (mid + m >= end
        || (start + s < mid
         && eventsInOrder(this->array[start + s], this->array[mid + m]))) {
            result[i] = this->array[start + s];
            s++;
        } else {
            result[i] = this->array[mid + m];
            m++;
        }
    }

    memcpy(this->array + start, result, resultSize);

    free(result);
}

bool eventsInOrder(Event *a, Event *b) {
    if (a->currentTime < b->currentTime) {
        return true;
    } else if (a->currentTime > b->currentTime) {
        return false;
    } else if (a->type == FINISHED && b->type == RUNNING) {
        return true;
    } else if (a->type == RUNNING && b->type == FINISHED) {
        return false;
    } else if (a->type == RUNNING && b->type == RUNNING) {
        return a->cpu < b->cpu;
    } else if (a->type == FINISHED && b->type == FINISHED) {
        return true; /* tie break later */
    } else {
        printf("Event type error");
        return true;
    }
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