#ifndef MCP_H
#define MCP_H


#include <iostream>
#include <string.h>
#include "Sema.h"
#include "ipc.h"
#include "Sched.h"
#include "MemMgr.h"
#include "ufs.h"

using namespace std;

//Master Control Block
class MCB {

public:
	//constructor
	MCB();

	//destructor
	~MCB();

	Scheduler Swapper;
	ipc	Messenger;	//object that provides access to thread ipc objects	
	MemMgr* memoryManager; //memory manager
	ufs* fileSystem;

	//semaphore
	Semaphore*	Monitor;
	Semaphore*	Printer;

};

#endif