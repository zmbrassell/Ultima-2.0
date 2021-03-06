#include "ipc.h"

// Constructor
ipc::ipc() {
	taskMessageHeadNode = NULL;
	resource = new Semaphore("IPC", 1);
}

// destructor
ipc::~ipc() {

	//cout << "~ipc was called..." << endl;

	delete resource;

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		//cout << "messageDeleteAll was called..." << endl;

		messageDeleteAll(current->taskID);

		//cout << "messageDeleteAll was called successfully" << endl;

		//temp node to delete
		TaskMessageNode* temp = current;

		current = current->next;

		delete temp->taskResource;
		delete temp;

		//cout << "current != NULL" << endl;
	}

	//cout << "~ipc was called" << endl;
}

//send message
// returns -1 if error occurred. Return 1 if successful.
int ipc::messageSend(TCB* pTCB, Message *message) {

	//add current time
	message->MessageArrivalTime = time(NULL);//get current time

	//send to queue of destination
	int destinationID = message->DestinationTaskId;
	
	if (resource->down(pTCB)) {//request resource is succesful


		/////////////critcal section

		TaskMessageNode* node = NULL;

		//iterate pointer
		TaskMessageNode* current = taskMessageHeadNode;

		//iterate the linked list
		while (current != NULL)
		{
			if (current->taskID == destinationID)
			{
				node = current;
				break;
			}

			current = current->next;
		}

		if (node == NULL)
		{//add new node
			node = new TaskMessageNode();
			node->taskResource = new Semaphore("Task", 1);
			node->taskID = destinationID;
			node->next = NULL;

			if (taskMessageHeadNode == NULL)//add as head
			{
				taskMessageHeadNode = node;
			}
			else {//add as tail

				  //iterate pointer
				TaskMessageNode* tail = taskMessageHeadNode;

				//iterate the linked list
				while (tail->next != NULL)
				{
					tail = tail->next;
				}

				tail->next = node;
			}
		}

		/////////////end critcal section

		//release the resource
		resource->up(pTCB);

		///critial section for specific queue

		if (node->taskResource->down(pTCB)) {

			node->messageQueue.enqueue(message);

			node->taskResource->up(pTCB);
			//end critial section
		}
		else {
			return -1; //failed to send message
		}
	}
	else
		return -1; // failed to send message
				
	

	return 0;
}

/*receive message
returns 0 if no more messages are
available, loads the Message structure with
the first message from the mailbox and
remove the message from the mailbox.
Return -1 if an error occurs.
*/
int ipc::messageReceive(TCB* pTCB, Message *&message) {

	int success = 0;

	if (resource->down(pTCB)) {
		// request resource is succesful

			//while (pTCB->state != READY && pTCB->state != RUNNING) {
			//	sleep(1);
			//}

			/////////////critcal section

			TaskMessageNode * node = NULL;

		//iterate pointer
		TaskMessageNode* current = taskMessageHeadNode;

		//iterate the linked list
		while (current != NULL)
		{
			if (current->taskID == pTCB->id)
			{
				node = current;
				break;
			}

			current = current->next;
		}

		/////////////end critcal section

		//release the resource
		resource->up(pTCB);

		///critial section for specific queue
		if (node != NULL) {

			if (node->taskResource->down(pTCB)) {

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				if (node->messageQueue.isEmpty())
				{
					success = -1;
				}
				else {

					message = node->messageQueue.peek();
					node->messageQueue.dequeue();
				}

				node->taskResource->up(pTCB);
			}
		}
		else {
			success = -1;
		}

		//end critial section
	}
	else
	{
		success = -1;
	}


	return success;
}

// return the number of messages in Task-id's message queue.
int ipc::messageCount(int taskID) {

	//get node by task id
	TaskMessageNode* node = NULL;

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		if (current->taskID == taskID)
		{
			node = current;
			break;
		}

		current = current->next;
	}

	if (node != NULL)
	{
		return node->messageQueue.size();
	}
	else {
		return 0;
	}
}

// return the total number of messages in all the message queues.
int ipc::messageCount() {

	int totalMessages = 0; //total number of messages

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		totalMessages += current->messageQueue.size();

		current = current->next;
	}

	return totalMessages;
}

// print the all messages for a given Task-id.
void ipc::messagePrint(int taskID, char largeBuffer[], WINDOW * Win) {

    stringstream ss;
  
	//get node by task id
	TaskMessageNode* node = NULL;

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		if (current->taskID == taskID)
		{
			node = current;
			break;
		}

		current = current->next;
	}
	

	if (node != NULL)
	{
		ss << endl;
		ss << " Task Number:   " << taskID << endl;
		ss << " Message Count: " << messageCount(taskID) << endl;
		ss << " Mail Box:" << endl;

		ss << left << setw(22) << " Source Task-id" << 
			setw(22) << "Destination Task-id" <<
			setw(30) << "Message Content" <<
			setw(15) << "Message size" <<
			setw(15) << "Message Type" <<
			setw(23) << "Message Arrival Time" << endl;

		Message** messageArray = new Message*[node->messageQueue.size()];

		//for testing purpose (dump)
		node->messageQueue.getArray(messageArray);

		for (size_t i = 0; i < node->messageQueue.size(); i++)
		{
			ss << " " << setw(22) << messageArray[i]->SourceTaskId <<
				setw(22) << messageArray[i]->DestinationTaskId <<
				setw(30) << messageArray[i]->MsgText <<
				setw(15) << messageArray[i]->MsgSize <<
				setw(15) << messageArray[i]->MsgType.MessageTypeDescription <<
				setw(23) << ctime(&messageArray[i]->MessageArrivalTime) << endl;
		}
		ss << endl;
		//copy to buffer
		strcpy(largeBuffer, ss.str().c_str());

		//print the buffer to the tasks window
		write_window(Win, largeBuffer);
		sleep(1); // so it stays visible for at least a little bit

		//free array
		delete [] messageArray;
	}

}

// delete all the messages for Task_id
int ipc::messageDeleteAll(int taskID) {

	TaskMessageNode* node = NULL;

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		if (current->taskID == taskID)
		{
			node = current;
			break;
		}

		current = current->next;
	}

	//remove queue
	if (node != NULL)
	{
		node->messageQueue.clear();
	}

	return 0;
}

/*
print all the messages in the message queue, but
do not delete them from the queue.
(note that this function may be best placed in the	scheduler!*/
void ipc::IPCMessageDump(char largeBuffer[], WINDOW * Win) {

	//iterate pointer
	TaskMessageNode* current = taskMessageHeadNode;

	//iterate the linked list
	while (current != NULL)
	{
		//print queue
	     messagePrint(current->taskID, largeBuffer, Win);

		current = current->next;
	}
}
