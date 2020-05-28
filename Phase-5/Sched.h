﻿#ifndef SCHEDULER_H
#define SCHEDULER_H


#include <iostream>
#include <iomanip>
#include <string.h>
#include "TCB.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


using namespace std;


//define Scheduler class
class Scheduler
{
public:
    //constructor
    Scheduler();


    //destructor
    ~Scheduler();


    // create appropriate data structures and calls coroutine()
    TCB* createTask(const char* taskName);


    // to kill a task (Set its status to DEAD)
    void destroyTask(int id);


    // strict round robin process switch.
    void yield();

	// returns the current Task-ID
	int get_task_id();


    // remove dead task, free their resources, etc.
    void garbage_collect();    


    // debugging function with level indicating the verbosity of the dump include some
    //functions which will allow you to dump the contents of the process table in a readable format.
  void dump(int level, char largeBuffer[]);    

  TCB *processTable;
private:


    int currentID; //current ID to generate the task ID
    int numElements; //number of elements in list
    int currentQuantum;
    

	double schedTime()
	{
		return ((double)time(NULL));
	}

};


#endif
