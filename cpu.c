#include "cpu.h"

struct SubprocessNode { /* this will be managed by CPU */
    Subprocess *subprocess; /* cpu only run subprocesses */
    SubprocessNode *next;
};

Subprocess *popHead(CPU *this);

CPU *newCPU(int id) {
    CPU *this = malloc(sizeof(CPU));
    assert(this != NULL);
    this->remainingQueueTime = 0;
    this->head = NULL;
    this->id = id;
    this->processRunning = false;
    return this;
}

Events *finishAllProcesses(CPU *this, Time time) {
    Events *events = newEvents();

    while (this->head != NULL) {
        Subprocess *run = this->head->subprocess;
        if (!this->processRunning) {
            addRunning(events, time, run, this->id);
            this->processRunning = true;
        }
        time += run->remainingTime;
        run->remainingTime = 0; 
        recordSubprocessFinished(run->parent, time, this->id);
        if (isFinished(run->parent)) {
            addFinished(events, time, run->parent);
        }
        popHead(this);
        this->processRunning = false;
    }
    return events;
}

Events *elapseTime(CPU *this, Time lastTime, Time currentTime) {

    Events *events = newEvents();

    Time toBeElapsed = currentTime - lastTime;

    while (this->head != NULL && toBeElapsed != 0) {

        /* elapse time for running process */
        Subprocess *running = this->head->subprocess;
        /* start if not already running */
        if (!this->processRunning) {
            addRunning(events, currentTime - toBeElapsed, running, this->id);
            this->processRunning = true;
        }

        if (running->remainingTime > toBeElapsed) {
            running->remainingTime -= toBeElapsed;
            this->remainingQueueTime -= toBeElapsed;
            toBeElapsed = 0;
        } else {
            toBeElapsed -= running->remainingTime;
            this->remainingQueueTime -= running->remainingTime;
            running->remainingTime = 0;

            /* finished, remove from queue */
            popHead(this);
            this->processRunning = false;

            /* check parent finished */
            recordSubprocessFinished(running->parent, currentTime - toBeElapsed, this->id);
            if (isFinished(running->parent)) {
                addFinished(events, currentTime - toBeElapsed, running->parent);
            }
        }
    }

    return events;
}

void addToQueue(CPU *this, Subprocess* toAdd) {
    
    this->remainingQueueTime += toAdd->remainingTime;

    /* insert into the correct place */
    SubprocessNode *walk = this->head;
    SubprocessNode *previous = NULL;
    while (walk != NULL) {
        if (toAdd->remainingTime < walk->subprocess->remainingTime
        || (toAdd->remainingTime == walk->subprocess->remainingTime
         && toAdd->parent->id < walk->subprocess->parent->id)) {
            break;
        }
        previous = walk;
        walk = walk->next;
    }

    /* insert before walk */
    SubprocessNode *node = malloc(sizeof(SubprocessNode));
    assert(node != NULL);
    node->subprocess = toAdd;

    node->next = walk;

    if (previous != NULL) {
        previous->next = node;
    } else {
        this->head = node;
    }

    if (this->processRunning && toAdd == this->head->subprocess) {
        /* if toAdd got added to the head and will replace current process */
        this->processRunning = false;
    }
}

Subprocess *popHead(CPU *this) {
    if (this->head == NULL) {
        printf("nothing to remove.");
    }
    Subprocess *result = this->head->subprocess;
    SubprocessNode *previous = this->head;
    this->head = this->head->next;
    free(previous);
    return result;
}

void destroyCPU(CPU *this) {
    /* free queue */
    SubprocessNode *walk = this->head;
    while (walk != NULL) {
        SubprocessNode *nextWalk = walk->next;
        free(walk);
        walk = nextWalk;
    }
    free(this);
}