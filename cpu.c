#include "cpu.h"

struct SubprocessNode { /* this will be managed by CPU */
    Subprocess *subprocess; /* cpu only run subprocesses */
    SubprocessNode *next;
};

void addToQueue(CPU *this, Subprocess* toAdd);
Subprocess *popHead(CPU *this);

CPU *newCPU(int id) {
    CPU *this = malloc(sizeof(CPU));
    assert(this != NULL);
    this->remainingQueueTime = 0;
    this->head = NULL;
    this->id = id;
    return this;
}

Events *finishAllProcesses(CPU *this, Time time) {
    Events *events = newEvents();
    while (this->head != NULL) {
        Subprocess *run = this->head->subprocess;
        time += run->remainingTime;
        run->remainingTime = 0;
        if (isFinished(run->parent)) {
            addFinished(events, time, run->parent);
        }
        popHead(this);
    }
    return events;
}

Events *elapseTimeAndAddToQueue(CPU *this, Time lastTime, Subprocess *toAdd, Time addTime) {

    Events *events = newEvents();

    Time toBeElapsed = addTime - lastTime;

    bool unfinished = false;
    while (this->head != NULL) {

        /* elapse time for running process */
        Subprocess *running = this->head->subprocess;
        if (running->remainingTime > toBeElapsed) {
            running->remainingTime -= toBeElapsed;
            this->remainingQueueTime -= toBeElapsed;
            toBeElapsed = 0;

            /* new process added in a middle of process running */
            unfinished = true;
            break;
        } else {
            toBeElapsed -= running->remainingTime;
            this->remainingQueueTime -= running->remainingTime;
            running->remainingTime = 0;

            /* finished, remove from queue */
            popHead(this);

            /* check parent finished */
            if (isFinished(running->parent)) {
                addFinished(events, addTime - toBeElapsed, running->parent);
            }

            /* is there a next process to start? */
            if (this->head != NULL && toBeElapsed != 0) {
                addRunning(events, addTime - toBeElapsed, this->head->subprocess, this->id);
            } else {
                break;
            }
        }
    }
    
    addToQueue(this, toAdd);

    if (unfinished && toAdd != this->head->subprocess) {
        /* no new event */
    } else {
        addRunning(events, addTime, this->head->subprocess, this->id);
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
    node->subprocess = toAdd;

    node->next = walk;

    if (previous != NULL) {
        previous->next = node;
    } else {
        this->head = node;
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