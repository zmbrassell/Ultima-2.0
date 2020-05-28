#ifndef TCB_H
#define TCB_H

#include <time.h>
#include <ncurses.h>
#define TCB_NAME_SIZE 64


//State of the task
enum STATE
{
    RUNNING, READY, BLOCKED, DEAD
};


//task control table
struct TCB {
    int id;          //task id
    char name[TCB_NAME_SIZE]; //name of task
    STATE state;
    double startTime;
    WINDOW * taskWin;
    TCB* next; //next node in linked list
};


#endif
