#ifndef CPU
#define CPU

#include "head.h"
#include "events.h"
#include "process.h"

typedef struct CPU CPU;
typedef struct SubprocessQueue SubprocessQueue;
typedef struct SubprocessNode SubprocessNode;

struct CPU { /* an OOP struct */
    SubprocessQueue queue;
    Time remainingQueueTime;
    int id;
};

struct SubprocessQueue { /* this will be managed by CPU */
    SubprocessNode *head;
    SubprocessNode *tail;
};

struct SubprocessNode { /* this will be managed by CPU */
    Subprocess *subprocess; /* cpu only run subprocesses */
    SubprocessNode *previous;
    SubprocessNode *next;
};

CPU *newCPU() {
    CPU *this = malloc(sizeof(CPU));
    this->remainingQueueTime = 0;
    this->queue.head = NULL;
    this->queue.tail = NULL;
    return this;
}

Events *elapseTimeAndAddToQueue(CPU *this, Time lastTime, Subprocess *toAdd, Time addTime) {

    Events *events = newEvents();

    Time toBeElapsed = addTime - lastTime;
    bool unfinished = false;
    while (true) {

        /* elapse time for running process */
        Subprocess *running = this->queue.head->subprocess;
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
            removeQueueHead(this);

            /* check parent finished */
            if (isFinished(running->parent)) {
                addFinished(events, addTime - toBeElapsed, running->parent);
            }

            /* is there a next process to start? */
            if (this->queue.head != NULL && toBeElapsed != 0) {
                addRunning(events, addTime - toBeElapsed, this->queue.head->subprocess, this->id);
            } else {
                break;
            }
        }
    }
    addToQueue(this, toAdd);
    if (unfinished && toAdd != this->queue.head->subprocess) {
        /* no new event */
    } else {
        addRunning(events, addTime, this->queue.head->subprocess, this->id);
    }
    return events;
}

void addToQueue(CPU *this, Subprocess* toAdd) {
    /* insert into the correct place */
    SubprocessNode *walk = this->queue.head;
    while (walk != NULL) {
        if (toAdd->remainingTime < walk->remaningTime
        || (toAdd->remainingTime == walk->remainingTime
         && toAdd->parent->id < walk->parent->id)) {
            break;
        }
    }

    /* insert before walk */
    SubprocessNode *node = malloc(sizeof(SubprocessNode));
    node->data = toAdd;

    SubprocessNode *previous;
    if (walk != NULL) {
        previous = walk->previous;
        walk->previous = node;
    } else {
        previous = this->queue.tail;
    }
    node->next = walk;

    if (previous != NULL) {
        previous->next = node;
    }
    node->previous = previous;
}

void removeQueueHead(CPU *this) {
    if (this->queue.head == NULL) {
        printf("nothing to remove.");
    }
    this->queue.head = this->queue.head->next;
    if (this->queue.head == NULL) {
        this->queue.tail = NULL;
    }
    /* don't have to free here as Subprocess is destroyed by Process */
}

void destroyCPU(CPU *this) {
    /* free queue */
    SubprocessNode *walk = this->queue.head;
    while (walk != NULL) {
        SubprocessNode *nextWalk = walk->next;
        free(walk);
        walk = nextWalk;
    }
    free(this);
}

#endif