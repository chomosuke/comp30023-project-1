#include "cpu.h"

void addToQueue(CPU *this, Subprocess* toAdd);
Subprocess *popHead(CPU *this);
void pushHead(CPU *this, Subprocess *subprocess);

CPU *newCPU() {
    CPU *this = malloc(sizeof(CPU));
    this->remainingQueueTime = 0;
    this->head = NULL;
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
    while (true) {

        /* elapse time for running process */
        Subprocess *running = this->head->subprocess;
        if (running->remainingTime > toBeElapsed) {
            running->remainingTime -= toBeElapsed;
            this->remainingQueueTime -= toBeElapsed;

            /* new process added in a middle of process running */
            unfinished = true;
            break;
        } else {
            toBeElapsed -= running->remainingTime;
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

    // if (unfinished && toAdd == this->head->subprocess
    //  && this->head->next != NULL
    //  && toAdd->remainingTime == this->head->next->subprocess->remainingTime) {
    //     /* swap */
    //     Subprocess *second = popHead(this);
    //     Subprocess *first = popHead(this);
    //     pushHead(this, second);
    //     pushHead(this, first);
    // }

    if (unfinished && toAdd != this->head->subprocess) {
        /* no new event */
    } else {
        addRunning(events, addTime, this->head->subprocess, this->id);
    }
    return events;
}

void addToQueue(CPU *this, Subprocess* toAdd) {
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
    }
}

Subprocess *popHead(CPU *this) {
    if (this->head == NULL) {
        printf("nothing to remove.");
    }
    Subprocess *result = this->head->subprocess;
    free(this->head);
    this->head = this->head->next;
    return result;
}

void pushHead(CPU *this, Subprocess *subprocess) {
    SubprocessNode *node = malloc(sizeof(SubprocessNode));
    node->subprocess = subprocess;
    SubprocessNode *previousHead = this->head;
    this->head = node;
    node->next = previousHead;
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