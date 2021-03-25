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

Events *elapseTime(CPU *this, Time lastTime, Time now) {
}

Events *elapseTimeAndAddToQueue(CPU *this, Time lastTime, Subprocess *toAdd, Time addTime) {
    Time elapsed = addTime - lastTime;
    while (this->queue.head != NULL) {

        /* elapse time for running process */
        Subprocess *running = this->queue.head->subprocess;
        if (running->remainingTime >= elapsed) {
            running->remainingTime -= elapsed;
            this->remainingQueueTime -= elapsed;

            if (running->remainingTime == 0) {
                /* process finish as new process is added */
            } else {
                /* new process added in a middle of process running */
            }

            break;
        } else {
            /* finished, remove from queue */
            removeQueueHead(this);

            /* update elapsed time for upcoming process */
            elapsed = -running->remainingTime;
            running->remainingTime = 0; /* for con */
            /* start next */
        }
    }
}

void addToQueue(CPU *this, Subprocess* process) {
    /* insert into the correct place */
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