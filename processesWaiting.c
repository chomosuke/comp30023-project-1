#include "processesWaiting.h"

ProcessesWaiting *newProcessesWaiting() {
    ProcessesWaiting *this = malloc(sizeof(ProcessesWaiting));
    assert(this != NULL);

    this->biggest = NULL;
    this->smallest = NULL;

    return this;
}

Process *popBiggest(ProcessesWaiting *this) {
    ProcessNode *node = this->biggest;
    if (node == NULL) {
        return NULL;
    }
    Process *result = node->process;
    this->biggest = node->smaller;
    if (this->biggest != NULL) {
        this->biggest->bigger = NULL;
    } else { /* list empty */
        this->smallest = NULL;
    }
    free(node);
    return result;
}

Process *popSmallest(ProcessesWaiting *this) {
    ProcessNode *node = this->smallest;
    if (node == NULL) {
        return NULL;
    }
    Process *result = node->process;
    this->smallest = node->bigger;
    if (this->smallest != NULL) {
        this->smallest->smaller = NULL;
    } else { /* list empty */
        this->biggest = NULL;
    }
    free(node);
    return result;
}

void addProcess(ProcessesWaiting *this, Process *process) {
    /* start with smallest and insert */
    ProcessNode *node = malloc(sizeof(ProcessNode));
    assert(node != NULL);
    node->process = process;
    node->bigger = this->smallest; /* assume process is the smallest */
    node->smaller = NULL;
    while (node->bigger != NULL) {
        if (node->bigger->process->exeTime > node->process->exeTime) {
            break;
        } else { /* if bigger is smaller, change bigger to something bigger */
            node->smaller = node->bigger;
            node->bigger = node->bigger->bigger;
        }
    }
    if (node->bigger == NULL) {
        /* end of list */
        this->biggest = node;
    } else {
        /* mid of list, connect */
        node->bigger->smaller = node;
    }
    if (node->smaller == NULL) {
        this->smallest = node;
    } else {
        node->smaller->bigger = node;
    }
}

bool noProcess(ProcessesWaiting *this) {
    return this->biggest == NULL;
}

void destoryProcess(ProcessesWaiting *this) {
    /* free all the nodes */
    while (this->biggest != NULL) {
        ProcessNode *node = this->biggest;
        this->biggest = this->biggest->smaller;
        free(node);
    }
    free(this);
}