#ifndef IPC_H
#define IPC_H

#include <iostream>
#include <iomanip>
#include <ctime>
#include "Queue.h"
#include "Sema.h"
#include "TCB.h"
#include "Ultima.h"
using namespace std;

//length of string in description of MessageType structure
#define MESSAGE_TYPE_DESC_LEN 64

//length of string in text of Message structure
#define MESSAGE_TEXT_LEN 32

/*Message type*/
struct MessageType {

	/*
	0 = text(not action required)
	1 = service(request for a service by the task, send a notification or result of the service back.)
	2 = Notification(sent to a task to indicate a service was completed.No action is required.)
	*/
	int MessageTypeId;
	char MessageTypeDescription[MESSAGE_TYPE_DESC_LEN];
};

/*Message structure*/
struct Message {
	//ID source of source task
	int SourceTaskId;

	//ID of destination task
	int DestinationTaskId;

	//arrival time of message
	time_t MessageArrivalTime;	// research time.h, time_t, and tm

	//type of message
	MessageType MsgType;

	//message size
	int MsgSize;	// lets make these	up to 32 bytes long
	
	//message text
	char MsgText[MESSAGE_TEXT_LEN];
};

//linked list of {task id with queue of messages}
struct TaskMessageNode {
	int taskID;          //task id
	Queue<Message*> messageQueue;
	Semaphore* taskResource; //resource of this task

	TaskMessageNode* next; //next node in linked list
};


/*ipc class*/
class ipc {	

public:		
	// Constructor
	ipc();	

	// destructor
	~ipc();

	//send message
	int messageSend(TCB* pTCB, Message *message);	// returns -1 if error occurred. Return 1 if successful.

	/*receive message
	 returns 0 if no more messages are
	 available, loads the Message structure with
	 the first message from the mailbox and
	 remove the message from the mailbox.
	 Return - 1 if an error occurs.
	*/
	int messageReceive(TCB* pTCB, Message *&message);

	// return the number of messages in Task-id's message queue.
	int messageCount(int taskID);

	// return the total number of messages in all the	message queues.
	int messageCount();	

	// print the all messages for a given Task-id.
    void messagePrint(int taskID, char largeBuffer[], WINDOW * Win);

	// delete all the messages for Task_id
	int messageDeleteAll(int taskID);

	/*
	print all the messages in the message queue, but
	do not delete them from the queue. 
	(note that this function may be best placed in the	scheduler!*/
    void IPCMessageDump(char largeBuffer[], WINDOW * Win);
private:

	//head node of linked list of TaskMessageNode
	TaskMessageNode* taskMessageHeadNode;

	//create semaphore to protect resource (linked list of nodes)
	Semaphore* resource;
};


#endif
