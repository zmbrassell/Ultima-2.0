#include "Sched.h"
#include <string.h>
#include <iomanip>
#include <iostream>
#include "Ultima.h"
using namespace std;



//constructor
Scheduler::Scheduler()
{
	processTable = NULL;
	currentID = 0;
	numElements = 0;
	currentQuantum = 5;
}

//destructor
Scheduler::~Scheduler()
{
	//pointer to iterate the linked list
	TCB *current = processTable;


	//loop and delete
	while (current != NULL)
	{
		TCB *temp = current;
		current = current->next;

		delete temp;
	}
}

//Return current task id
int Scheduler::get_task_id()
{
	return currentID;
}

// create appropriate data structures and calls coroutine()
TCB* Scheduler::createTask(const char* taskName) {


	//create new TCB
	TCB* newTCB = new TCB;
	newTCB->id = currentID++;
	strcpy(newTCB->name, taskName);
	newTCB->state = READY;
	newTCB->next = NULL;


	if (processTable == NULL)//linked list is empty
	{
		processTable = newTCB;
		numElements++;
	}
	else {//add as tail


		TCB* processTail = processTable;
		while (processTail->next != NULL)
		{
			processTail = processTail->next;
		}


		processTail->next = newTCB;
	}


	return newTCB;
}


// to kill a task (Set its status to DEAD)
void Scheduler::destroyTask(int id) {


	//pointer to iterate the linked list
	TCB *current = processTable;


	//loop and find id
	while (current != NULL)
	{
		if (current->id == id)
		{
			current->state = DEAD;
			numElements--;
			break;
		}
		current = current->next;
	}
}


// strict round robin process switch.
void Scheduler::yield() {
	if (processTable != NULL)
	{
	        char buff[256];
		TCB* current = processTable;

		/*if (current->state == DEAD) {
		  sprintf(buff, " ATTEMPTING TO YIELD\n");
		  write_window(current->taskWin, buff);
			//cout << current->name << " is trying to yield... " << endl;
		}
		*/
	       
		/*while (current->state == DEAD) {
		  sprintf(buff, " ****ALREADY DEAD****\n");
		  write_window(current->taskWin, buff);
		  current = current->next;
			//cout << current->name << " is already dead! Yielding now..." << endl;
		  }
		*/
		
		// Check to see if it's time to yield.
		//if ((schedTime() - current->startTime) >= currentQuantum)
		//{
			for (size_t i = 0; i < numElements; i++)
			{
				if (current->state == READY)
				{//allow head to run
				        
					//cout << processTable->name << " is running..." << endl;
					processTable->state = RUNNING;
					processTable->startTime = schedTime();
					//sprintf(buff, " **** CURRENTLY RUNNING *****\n");
					//write_window(processTable->taskWin, buff);
					break;
				}
				else{
					//move head to back
				  
				  TCB* processTail = processTable;
				  sprintf(buff, " **** ATTEMPTING TO YIELD ****\n");
				  write_window(processTable->taskWin, buff);
									     
					while (processTail->next != NULL)
					{
						processTail = processTail->next;
					}

					if((processTable->state == RUNNING && (schedTime() - current->startTime) >= currentQuantum) || processTable->state == DEAD || processTable->state == BLOCKED){
					  if (processTable != processTail)//more than 2 elements
					    {

						TCB* nextHead = processTable->next; //next of head
			    
						if(processTable->state == RUNNING){
							processTable->state = READY;
						}
						else if(processTable->state == DEAD)
						  {
						    sprintf(buff, " **** ALREADY DEAD, YIELDING NOW ****\n");
						    write_window(processTable->taskWin, buff);
						    processTable->state = DEAD;
						  }
						else if(processTable->state == BLOCKED)
						  {
						    sprintf(buff, " **** BLOCKED, YIELDING NOW ****\n");
						    write_window(processTable->taskWin, buff);
						  }
						
						sprintf(buff, " **** SUCCESFULLY YIELDED ****\n");
					       	write_window(processTable->taskWin, buff);
					      

						//head become tail
						processTail->next = processTable;
						processTable->next = NULL;


						processTable = nextHead;
						//cout << "Yield successful..." << endl;
					    }
					}
				}
			}
			//}
			//else {
			//cout << current->name << " still running..." << endl;
			//}
	}

}


// remove dead task, free their resources, etc.
void Scheduler::garbage_collect() {


	//pointer to iterate the linked list
	TCB *current = processTable; 
	TCB *previous = NULL;


	//loop and find id
	while (current != NULL)
	{
		if (current->state == DEAD)
		{
			if (processTable == current)//delete head?
			{
				processTable = current->next;
			}


			TCB *temp = current;
			current = current->next;
			delete temp;


			numElements--;
		}
		else {
			current = current->next;
		}
	}


}


// debugging function with level indicating the verbosity of the dump include some
//functions which will allow you to dump the contents of the process table in a readable format.
void Scheduler::dump(int level, char largeBuffer[]) {

  stringstream ss;
	ss << endl << "------------------------------------------" << endl;
	ss << " Scheduler Dump" << endl;
	ss << " Quantum: " << currentQuantum << endl;
	ss << " Time elapsed for current task: " << std::fixed << (int)(schedTime() - processTable->startTime) << endl;

	ss << setw(13) << " Task Name" << setw(13) << "Task ID" << setw(13) << "State" << endl;


	//pointer to iterate the linked list
	TCB *current = processTable;


	//loop and find id
	while (current != NULL)
	{
		ss << setw(13) << current->name << setw(13) << current->id;

		if (current->state == READY)
		{
			ss << setw(13) << "READY" << endl;
		}
		else if (current->state == BLOCKED)
		{
			ss << setw(13) << "BLOCKED" << endl;
		}
		else if (current->state == DEAD)
		{
			ss << setw(13) << "DEAD" << endl;
		}
		else if (current->state == RUNNING)
		{
			ss << setw(13) << "RUNNING" << endl;
		}


		current = current->next;
	}
	ss << "------------------------------------------" << endl;

	//copy to buffer
	strcpy(largeBuffer, ss.str().c_str());
}
