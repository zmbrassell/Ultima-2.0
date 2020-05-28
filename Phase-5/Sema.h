﻿#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <string.h>
#include <iostream>
#include "Queue.h"
#include "TCB.h"
#include "Sched.h"
#include "Ultima.h"
using namespace std;


#define RESOURCE_NAME_SIZE 64


class Semaphore
{
public:


    //constructor
  Semaphore(const char* resourceName, int semaVal);


    //destructor
    ~Semaphore();
    
    bool down(TCB*);    // get the resource or get queued!


    // release the resource
    void up(TCB*);


    // include some functions which will allow you to dump the contents of the semaphore in
    //a readable format
  void dump(int level, char largeBuffer[]);


private:


    // the name of the resource being managed
    char resourceName[RESOURCE_NAME_SIZE];


    // 0 or 1 in the case of a binary semaphore
    volatile int semaValue;


    //sema queue
    Queue<TCB*> semaQueue;


    //to protect the semaValue
    pthread_mutex_t semMutex;

   //pointer to the scheduler
    Scheduler *schedPtr;

  // Task-id of the task with the resource
  int currentTask;
};




#endif
