#include "MemMgr.h"

// allocate 1024 unsigned chars and initialize the entire
//memory with "." dots.
MemMgr::MemMgr(int size, char default_initial_value){
	
	handleNum = 0;
	content = new char[size];

	for (int i = 0; i < size; i++)
	{
		content[i] = default_initial_value;
	}

	head = new Node();
	head->baseAddress = 0;
	head->size = size;
	head->status = 0; //free
	head->limitAddress = size - 1;
	head->currentLocation = 0;
	head->memoryHandle = ++handleNum;
	head->pTCB = NULL;
	head->next = NULL;

	resource = new Semaphore("MM", 1);
}

// returns a unique integer memory_handle or -1 if not enough
//memory is available. Set the Current_Location for this
//memory segment. (beginning of the allocated area)
int MemMgr::Mem_Alloc(TCB* pTCB, int size) {

	if (size % BLOCK_SIZE != 0){
	  size = (size / BLOCK_SIZE + 1) * BLOCK_SIZE;
     }
	int ret = -1;


			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section

				//find first free block
				Node* current = head;
				while (current != NULL)
				{
					if (current->status == 0 && size <= current->size)//is freed?
					{
						if (size == current->size)
						{
							current->status = 1;//is used
							current->memoryHandle = ++handleNum;
							current->currentLocation = 0;
							current->pTCB = pTCB;
							ret = current->memoryHandle;

						}else{//split node

							Node* remainingNode = new Node();
							remainingNode->baseAddress = current->baseAddress + size;
							remainingNode->size = current->size - size;
							remainingNode->status = 0; //free
							remainingNode->limitAddress = size - 1;
							remainingNode->currentLocation = 0;
							remainingNode->memoryHandle = ++handleNum;
							remainingNode->next = current->next;

							current->next = remainingNode;
							current->status = 1;//is used
							current->memoryHandle = ++handleNum;
							current->currentLocation = 0;
							current->size = size;
							current->limitAddress = current->baseAddress + size - 1;
							current->pTCB = pTCB;

							ret = current->memoryHandle;
						}

						break;
					}

					current = current->next;
				}
				/////////////end critcal section

				//release the resource
				resource->up(pTCB);

			}


	return ret;//not enough memory or all used or memory handle
}

// place "#" in the memory freed. Return -1 if error occurs
int MemMgr::Mem_Free(TCB* pTCB, int memory_handle){


	char buff[256];
	int ret = 0; //return value

			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section
				Node* node = getNode(memory_handle);

				if (node == NULL)
				{
					ret = -1;
				}else{

					for (int i = node->baseAddress; i <= node->limitAddress; i++)
					{
						content[i] = '#';
					}
					node->status = 0;//free
					node->currentLocation = node->baseAddress;
					node->pTCB = NULL;
				}


				/////////////end critcal section

				//release the resource
				resource->up(pTCB);
			}
				
				//attempt to coalesce everytime Mem_Free is called
			    Mem_Coalesce();
				

	return ret;
}

// combine two or more contiguous blocks of free space, and place "." Dots in the coalesced memory.
int MemMgr::Mem_Coalesce(){
	   int difference = 0;
		bool found = true;
		while (found)
		{
			found = false;

			//iterate and find two contiguous blocks of freed nodes
			Node* current = head;
			while (current != NULL && current->next != NULL)
			{
				Node* next = current->next;

				//both are free
				if (current->status == 0 && next->status == 0)
				{
					current->limitAddress = next->limitAddress;
					current->next = next->next;
					current->size = current->size + next->size;


					for (int i = current->baseAddress; i <= current->limitAddress; i++)
					{
						content[i] = '.';
					}

					found = true;

					if (current->size > RAM_SIZE) { // there was a bug resulting in memory chunks larger than the RAM_SIZE
									// this block attempts to remedy that bug
						difference = current->size - RAM_SIZE;
						current->size = current->size - difference;

					}
					
					


					//delete next; //without this commented out, a seg fault is caused when the next task tries to access memory

					break;//break inner while to start again
				}
			}
		} 
}

//get node  by memory handle
Node* MemMgr::getNode(int memory_handle){//get a node by memory handle

	//iterate and find node 
	Node* current = head;
	while (current != NULL)
	{
		if (current->memoryHandle == memory_handle)
		{
			return current;
		}

		current = current->next;
	}

	return NULL; //not found
}

// read a character from current location in memory and bring
//it back in "ch", return a -1 if at end of bounds. Keep track of
//the "Current Location" or the location of the next char to be read.
int MemMgr::Mem_Read(TCB* pTCB, int memory_handle, char *ch){


	int ret = 0; //return value

			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section

				//retrieve the node
				Node* node = getNode(memory_handle);

				if (node == NULL)
				{
					ret = -1;
				}else{

					//check the out of bound exception
					if (node->baseAddress + node->currentLocation <= node->limitAddress)
					{
						*ch = content[node->baseAddress + node->currentLocation++];
						
					}else{
						ret =  -1;
					}
				}

				/////////////end critcal section

				//release the resource
				resource->up(pTCB);

			}

	return ret;
}

// write a character to the current_location in memory, return
//a -1 if at end of bounds.
int MemMgr::Mem_Write(TCB* pTCB, int memory_handle, char ch){


	int ret = 0; //return value

			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section

				//retrieve the node
				Node* node = getNode(memory_handle);

				if (node == NULL)
				{
					ret = -1;
				}else{

					//check the out of bound exception
					if (node->baseAddress + node->currentLocation <= node->limitAddress)
					{
						content[node->baseAddress + node->currentLocation++] = ch;
					}else{
						ret = -1;
					}
				}
								
				/////////////end critcal section

				//release the resource
				resource->up(pTCB);
	
			}

	return ret;
}

//Overloaded multi-byte read and write.
int MemMgr::Mem_Read(TCB* pTCB, int memory_handle, int offset_from_beg, int text_size, char *text ){


	int ret = 0; //return value

			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section

				//retrieve the node
				Node* node = getNode(memory_handle);

				if (node == NULL)
				{
					ret = -1;
				}else{

					//check the out of bound exception
					if (node->baseAddress + offset_from_beg + text_size <= node->limitAddress)
					{
						strncpy(text, (content + node->baseAddress + offset_from_beg), text_size);
						text[text_size] = '\0';

						node->currentLocation = offset_from_beg + text_size;
					}else{
						ret = -1;
					}
				}

				/////////////end critcal section

				//release the resource
				resource->up(pTCB);

			}

	return ret;
}

//Write text to memory
int MemMgr::Mem_Write(TCB* pTCB, int memory_handle, int offset_from_beg, int text_size, const char *text ){

	int ret = 0; //return value

			if (resource->down(pTCB)) {//request resource is succesful

				/////////////critcal section
				//cout << "/////////////critcal section" << endl;

				//retrieve the node
				Node* node = getNode(memory_handle);

				if (node == NULL)
				{
					ret = -1;
				}else{

					//check the out of bound exception
					if (node->baseAddress + offset_from_beg + text_size <= node->limitAddress)
					{
						strncpy(content + node->baseAddress + offset_from_beg, text, text_size);
						node->currentLocation = offset_from_beg + text_size;
					}else{
						ret = -1;
					}
				}
				/////////////end critcal section
				//cout << "/////////////end critcal section" << endl;

				//release the resource
				resource->up(pTCB);

			}
		//}
	//}//end while
	return ret;
}

//destructor
MemMgr::~MemMgr()
{
	//iterate and delete node s
	Node* current = head;
	while (current != NULL)
	{
		Node* temp = current;

		current = current->next;

		delete temp;
	}

	delete [] content;
	delete resource;
}

// return the amount of core memory left in the OS
int MemMgr::Mem_Left(){

	int left = 0;//the amount of core memory left in the OS

	//iterate 
	Node* current = head;
	while (current != NULL)
	{
		if (current->status == 0) //is free
		{
			left += current->size;
		}
		current = current->next;
	}
	return left;
}

// return the size of largest available memory segment.
int MemMgr::Mem_Largest(){

	int largest = 0;//size of largest available memory segment

	//iterate 
	Node* current = head;
	while (current != NULL)
	{
		if (current->status == 0) //is free
		{
			if (current->size > largest){
				largest = current->size;
			}
		}
		current = current->next;
	}
	return largest;

}

// return the size of smallest available memory segment.
int MemMgr::Mem_Smallest(){

	int smallest = -1;//size of smallest available memory segment

	//iterate 
	Node* current = head;
	while (current != NULL)
	{
		if (current->status == 0) //is free
		{
			if (smallest == -1 || current->size < smallest){
				smallest = current->size;
			}
		}
		current = current->next;
	}
	return smallest;
}

// dump the contents of memory
int MemMgr::Mem_Dump(int starting_from, int num_bytes, char largeBuffer[]){
	int i = 0;

	stringstream ss;
	ss << endl;
	ss << " ";
	while (i < RAM_SIZE)
	{
		ss << content[i];
		
		i++;
		if (i % 128 == 0)
		{
			ss << endl;
			ss << " ";
		}
	}
	ss << endl;
	

	//copy to buffer
	strcpy(largeBuffer, ss.str().c_str());

}

//print Memory Usage
void MemMgr::printMemoryUsage(char largeBuffer[]){

	stringstream ss;
	ss << "\n Amount of core memory left: " << Mem_Left() << endl;
	ss << " Largest available memory segment: " << Mem_Largest() << endl;
	ss << " Smallest available memory segment: " << Mem_Smallest() << endl;

	ss << endl;
	ss << " Status    Memory    Starting    Ending     Size/     Current    Task-ID" << endl;
	ss << "           Handle    Location    Location   Bytes     Location            " << endl;



	//iterate
	Node* current = head;
	while (current != NULL)
	{
		if (current->status == 0)
		{
			ss << left << setw(10) << " Free";
		}else{ 
			ss << left << setw(10) << " Used";
		}

		ss << setw(10) << current->memoryHandle;
		ss << setw(12) << current->baseAddress;
		ss << setw(12) << current->limitAddress;
		ss << setw(10) << current->size;
		ss << setw(10) << current->currentLocation + current->baseAddress;

		if (current->status == 0)
		{
			ss << setw(10) << " MM";
		}else{
			ss << " " << setw(10) << current->pTCB->id;
		}

		ss << endl;

		current = current->next;
	}

	//copy to buffer
	strcpy(largeBuffer, ss.str().c_str());
}