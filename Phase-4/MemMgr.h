#ifndef MEM_MGR_H
#define MEM_MGR_H

#include "Sema.h"
#include "TCB.h"
#include "Ultima.h"
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#define RAM_SIZE 1024
#define BLOCK_SIZE 128

//Node represents the segment of memory
struct Node{
	int memoryHandle;
	int currentLocation;

	int status;  //0 is free, 1 is being used
	int size;    //in bytes = limit - base + 1
	int baseAddress;
	int limitAddress;

	TCB* pTCB;

	Node* next;
};

class MemMgr
{
public:
	// allocate 1024 unsigned chars and initialize the entire
	//memory with "." dots.
	MemMgr(int size, char default_initial_value);

	// returns a unique integer memory_handle or -1 if not enough
	//memory is available. Set the Current_Location for this
	//memory segment. (beginning of the allocated area)
	int Mem_Alloc(TCB* pTCB, int size);

	// place "#" in the memory freed. Return -1 if error occurs
	int Mem_Free(TCB* pTCB, int memory_handle);

	// read a character from current location in memory and bring
	//it back in "ch", return a -1 if at end of bounds. Keep track of
	//the "Current Location" or the location of the next char to be read.
	int Mem_Read(TCB* pTCB, int memory_handle, char *ch);

	// write a character to the current_location in memory, return
	//a -1 if at end of bounds.
	int Mem_Write(TCB* pTCB, int memory_handle, char ch);

	//Overloaded multi-byte read and write.
	int Mem_Read(TCB* pTCB, int memory_handle, int offset_from_beg, int text_size, char *text );
	int Mem_Write(TCB* pTCB, int memory_handle, int offset_from_beg, int text_size, const char *text );

	//destructor
	~MemMgr(void);

	//print Memory Usage
	void printMemoryUsage(char largeBuffer[]);

	// dump the contents of memory
	int Mem_Dump(int starting_from, int num_bytes, char largeBuffer[]);

	

private:

	// return the amount of core memory left in the OS
	int Mem_Left();
	// return the size of largest available memory segment.
	int Mem_Largest();
	// return the size of smallest available memory segment.
	int Mem_Smallest();
	// combine two or more contiguous blocks of free space, and place "." Dots in the coalesced memory.
	int Mem_Coalesce();
	

	Node* head;
	char* content;

	int handleNum; //handle number

	Node* getNode(int memory_handle);//get a node by memory handle

	//create semaphore to protect resource (linked list of nodes)
	Semaphore* resource;
};

#endif

