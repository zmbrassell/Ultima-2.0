﻿#include "Sema.h"
#include <pthread.h>


//constructor
Semaphore::Semaphore(const char* resame, int semaVal)
{
    semaValue = semaVal;
    strcpy(resourceName, resame);


    if (pthread_mutex_init(&semMutex, NULL) != 0) {
            cout << "Mutex init has failed" << endl;            
    }
}


//destructor
Semaphore::~Semaphore()
{
    pthread_mutex_destroy(&semMutex);
}


// get the resource or get queued!
bool Semaphore::down(TCB* pTCB) {


    bool success = false;

    // Lock the mutex
    pthread_mutex_lock(&semMutex);
    
    if(currentTask == pTCB->id)
    {//task calling down already has resource
       	success = true;
		semaValue = 0;
		pthread_mutex_unlock(&semMutex);
		return success; 
	
    }
      
    if (semaValue == 0)
    {
                char buff[256];
		pTCB->state = BLOCKED;
		sprintf(buff, "**** TASK WAS JUST BLOCKED ****\n");
		write_window(pTCB->taskWin, buff);
		semaQueue.enqueue(pTCB);

    }
    else if (semaValue == 1) {

		pTCB->state = RUNNING;
		semaValue = 0;
		success = true;
		currentTask = pTCB->id;
    }


    pthread_mutex_unlock(&semMutex);


    return success;
}


// release the resource
void Semaphore::up(TCB* pTCB) {


  // Lock mutex
    pthread_mutex_lock(&semMutex);    

    if(pTCB->id == currentTask){

    //set ready for TCB at top of queue
    if (!semaQueue.isEmpty())
    {
            TCB* pTCBHead = semaQueue.peek();
            semaQueue.dequeue();

	    
            if (pTCBHead->state != DEAD)
            {
                    pTCBHead->state = READY;
            }
	    semaValue = 1;
    }

	//if queue is empty, set to 1
	if (semaQueue.isEmpty()) {
		semaValue = 1;
	}
    }
    // Unlock Mutex
    pthread_mutex_unlock(&semMutex);
    
}


// include some functions which will allow you to dump the contents of the semaphore in
//a readable format
void Semaphore::dump(int level, char largeBuffer[]) {
  stringstream ss;
    ss << "------------------------------------------" << endl;
    ss << " Semaphore Dump" << endl;
    ss << " Resource: " << resourceName << endl;
    ss << " Sema Value: " << semaValue << endl;
    ss << " Sema queue:";


    TCB* tempArray[100];
    semaQueue.getArray(tempArray);


    for (size_t i = 0; i < semaQueue.size(); i++)
    {
            ss << " -> " << tempArray[i]->id;
    }
    ss << endl << "------------------------------------------" << endl << endl;

    //copy to buffer
    strcpy(largeBuffer, ss.str().c_str());
}
