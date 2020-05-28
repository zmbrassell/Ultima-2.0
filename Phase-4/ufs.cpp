#include "ufs.h"


//file system opeations
//constructor
ufs::ufs(string name, int noBlocks, int blockSize, char initChar) :
	fsName(name), fsNumBLocks(noBlocks), fsBlockSize(blockSize), initializationChar(initChar)
{
	inodes = new Inode[fsNumBLocks];	
	nextFileHandle = 0;

	resource = new Semaphore("INODE", 1);

	//format
	format();
}

//destructor
ufs::~ufs()
{
	delete[] inodes;	

	delete resource;
}

//format the current file system
void ufs::format() {

	//clear all inodes
	for (int i = 0; i < fsNumBLocks; i++)
	{
		memset((void*)&inodes[i], 0, sizeof(Inode));
	}

	//reset all file handle
	for (int i = 0; i < MAX_OPENED_FILES; i++)
	{
		memset((void*)&fileHandles[i], 0, sizeof(CurrentFile));
	}

	fstream afile(DATA_FILE, ios::out); //open file to write

	//initialize with initchar
	for (int i = 0; i < fsNumBLocks * fsBlockSize; i++)
	{
		afile << initializationChar;
	}

	afile.close(); //close file
}

//find free inode
int ufs::findFreeInode() {

	for (int i = 0; i < fsNumBLocks; i++)
	{
		if (strlen(inodes[i].filename) == 0)
		{
			return i;
		}
	}
	return -1;
}

//find contiguous file blocks
//return the starting block number
int ufs::findContigousFileBlocks(int numRequiredBlocks) {

	int bitmap = 0;
	for (size_t i = 0; i < fsNumBLocks; i++)
	{
		if (strlen(inodes[i].filename) != 0)
		{
			bitmap = bitmap | inodes[i].blocks;
		}

	}

	//starting bit and current bit
	int startingBits = -1;
	int currentBits = -1;

	for (int k = 0; k < fsBlockSize; k++)
	{
		if (((bitmap & (1 << k)) >> k) == 0)
		{
			if (startingBits == -1)
			{
				startingBits = k;
				currentBits = k;
			}
			else {
				currentBits = k;
			}

			if (currentBits - startingBits + 1 == numRequiredBlocks)
			{
				return startingBits;
			}
		}
		else {
			startingBits = -1;
			currentBits = -1;
		}
	}

	return -1;
}

//generate the bitmap that uses the freed blocks
//such as 0000 0000 0000 0000 ->got 4 freed blocks then put in i-node
int ufs::acquireBlocks(int startingBit, int numRequiredBlocks) {

	int bitmap = 0;

	for (size_t i = 0; i < numRequiredBlocks; i++)
	{
		bitmap = bitmap | (1 << (startingBit + i));
	}
	return bitmap;
}

/*
directory operations
precondition: fileSize > 0
permission is valid such as rwrw
*/
int ufs::createFile(TCB* pTCB, const char* filename, int fileSize, const char* permission) {

	int numResources = 0;

	int ret = 0; //return value
	//the task need 1 resource
	while (numResources < 1 && ret != -1)
	{

	  if (pTCB->state == RUNNING)
	  	{
		  if (resource->down(pTCB)) {//request resource is succesful

		  while (pTCB->state != READY && pTCB->state != RUNNING) {
		  	  		sleep(1);
		  	  	}

				/////////////critcal section
		                //find inode
				int currentIdx = findInodeIndex(filename);

				if (currentIdx != -1) //existing file
				{
					ret = -1;
				}

				if (ret == 0) {
					//find free inode
					int inodeIdx = findFreeInode();
					if (inodeIdx == -1)
					{
						ret = -1;

						//cout << "debug: free i node not found" << endl;
					}
					else {

						//calculate number of blocks to store file
						int numBlocks = fileSize / fsBlockSize;
						if (fileSize % fsBlockSize != 0)
						{
							numBlocks++;
						}

						if (numBlocks > 4) //maximum 4 blocks
						{
							ret = -1;
							//cout << "debug: maximum 4 blocks" << endl;
						}
						else {

							//find Contigous freed blocks
							int startingFreeBlockIdx = findContigousFileBlocks(numBlocks);

							if (startingFreeBlockIdx == -1)
							{
								ret = -1;
								//cout << "debug: findContigousFileBlocks not found" << endl;
							}
							else {

								//copy file name
								if (strlen(filename) > 7) //accept 8 characters including null character
								{
									strncpy(inodes[inodeIdx].filename, filename, 7);
									inodes[inodeIdx].filename[7] = '\0';
								}
								else {
									strcpy(inodes[inodeIdx].filename, filename);
								}
								inodes[inodeIdx].ownerTaskID = pTCB->id;
								inodes[inodeIdx].startingBlock = startingFreeBlockIdx;
								inodes[inodeIdx].blocks = acquireBlocks(startingFreeBlockIdx, numBlocks);
								inodes[inodeIdx].size = fileSize;

								strcpy(inodes[inodeIdx].permission, permission);
								time(&inodes[inodeIdx].creationTime);
								time(&inodes[inodeIdx].lastModifiedTime);
							}
						}
					}
				}
		   
				/////////////end critcal section
				
				//release the resource
				resource->up(pTCB);
				numResources++;
		  }
	       }
	  }

	return ret;
}

//file operations
//open file, return -1 if fail
//mode is r or write
//mode in inode is rwrw or rwrr or r w _ _
int ufs::open(TCB* pTCB, const char* filename, const char* mode) {

	int numResources = 0;

	int ret = 0; //return value

	//the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section
				//find inode
				int nodeIdx = findInodeIndex(filename);

				if (nodeIdx == -1) //file not found
				{
					ret = -1;
				}

				//get index of fileHandles by file name
				int currentFD = getCurrentFileStructure(filename);
				if (currentFD != -1) //file opened already
				{
					ret = -1;
				}

				//check mode in case not owner
				if (ret != -1 && inodes[nodeIdx].ownerTaskID != pTCB->id)
				{
					if (strcmp(mode, "r") == 0) //is read?
					{
						if (inodes[nodeIdx].permission[2] == '_')
						{
							ret = -1;//not allow to read
						}
					}
					else if (strcmp(mode, "w") == 0) //is write ?
					{
						if (inodes[nodeIdx].permission[3] == '_')
						{
							ret = -1;//not allow to read
						}
					}
				}

				if (ret != -1) {

					//current opened file?
					int idx = getFreedFileStructure();
					if (idx == -1)
					{
						ret = -1;
					}
					else {

						nextFileHandle++;

						fileHandles[idx].currentLocation = 0;
						fileHandles[idx].nodeID = nodeIdx;
						fileHandles[idx].taskID = pTCB->id;
						strcpy(fileHandles[idx].mode, mode); //r or w
						fileHandles[idx].fileHandle = nextFileHandle;

						ret = nextFileHandle;
					}
				}
				/////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

//find inode by task id and file name
int ufs::findInodeIndex(const char* filename) {

	for (int i = 0; i < fsNumBLocks; i++)
	{
		if (strcmp(inodes[i].filename, filename) == 0)
		{
			return i;
		}
	}
	return -1;
}

//get index of fileHandles by file handle
int ufs::getCurrentFileStructureByFD(int fileHandle) {
	for (int i = 0; i < MAX_OPENED_FILES; i++)
	{
		if (fileHandles[i].fileHandle == fileHandle)
		{
			return i;
		}
	}
	return -1;
}

//get freed CurrentFile structure in array
int ufs::getFreedFileStructure() {
	for (int i = 0; i < MAX_OPENED_FILES; i++)
	{
		if (fileHandles[i].fileHandle == 0)
		{
			return i;
		}
	}
	return -1;
}

//get index of fileHandles by file name
int ufs::getCurrentFileStructure(const char* filename) {

	for (int i = 0; i < MAX_OPENED_FILES; i++)
	{
		if (fileHandles[i].fileHandle != 0) {

			//compare file name
			if (strcmp(inodes[fileHandles[i].nodeID].filename, filename) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

//close file
int ufs::close(TCB* pTCB, int fileHandle) {
	
	int numResources = 0;

	int ret = 0; //return value

	//the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section

				//get index of fileHandles by file handle
				int idx = getCurrentFileStructureByFD(fileHandle);

				if (idx == -1)
				{
					ret = -1;
				}
				else {

					//not the task id
					if (fileHandles[idx].taskID != pTCB->id)
					{
						ret = -1;
					}
					else {

						//return the structure
						fileHandles[idx].currentLocation = 0;
						fileHandles[idx].nodeID = 0;
						fileHandles[idx].taskID = 0;
						strcpy(fileHandles[idx].mode, "");
						fileHandles[idx].fileHandle = 0;
					}
				}
				/////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

//read char, keep track current file location
int ufs::readChar(TCB* pTCB, int fileHandle, char* charContent) {

	int numResources = 0;

	int ret = 0; //return value

	//the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section
				//get index of fileHandles by file handle
				int idx = getCurrentFileStructureByFD(fileHandle);

				if (idx == -1)
				{
					ret = -1;
					//cout << "debug:  index of fileHandles by file handle" << endl;
				}
				else {

					//not the task id
					if (fileHandles[idx].taskID != pTCB->id)
					{
						ret = -1;
						//cout << "debug: not the task id" << endl;
					}
					else {

						//check location
						if (fileHandles[idx].currentLocation >= inodes[fileHandles[idx].nodeID].size)
						{
							ret = -1;//EOF
							//cout << "debug: check location" << endl;
						}
						else {

							int loc = inodes[fileHandles[idx].nodeID].startingBlock * fsBlockSize
								+ fileHandles[idx].currentLocation;
							fileHandles[idx].currentLocation++; //increase one location

							//cout << "inodes[fileHandles[idx].nodeID].startingBlock: " << inodes[fileHandles[idx].nodeID].startingBlock << endl;
							//cout << "fsBlockSize: " << fsBlockSize << endl;
							//cout << "fileHandles[idx].currentLocation: " << fileHandles[idx].currentLocation << endl;

							//copy data
							readCharFromFile(DATA_FILE, loc, charContent);
						}
					}
				}

				////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

//write a char
int ufs::writeChar(TCB* pTCB, int fileHandle, char charContent) {
	
	int numResources = 0;

	int ret = 0; //return value

	 //the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section
				//get index of fileHandles by file handle
				int idx = getCurrentFileStructureByFD(fileHandle);

				if (idx == -1)
				{
					ret = -1;
				}
				else {

					//not the task id
					if (fileHandles[idx].taskID != pTCB->id)
					{
						ret = -1;
					}
					else {

						//check location
						if (fileHandles[idx].currentLocation >= inodes[fileHandles[idx].nodeID].size)
						{
							ret = -1;//EOF
						}
						else {

							int loc = inodes[fileHandles[idx].nodeID].startingBlock * fsBlockSize
								+ fileHandles[idx].currentLocation;
							fileHandles[idx].currentLocation++; //increase one location

							//copy data
							writeCharToFile(DATA_FILE, loc, charContent);
						}
					}
				}

				////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

//delete file
int ufs::deleteFile(TCB* pTCB, const char* filename) {

	int numResources = 0;

	int ret = 0; //return value

				 //the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section

				//find inode
				int nodeIdx = findInodeIndex(filename);

				if (nodeIdx == -1) //file not found
				{
					ret = -1;
				}
				else {

					//file must be closed before deleting
					//get index of fileHandles by file name
					int currentFD = getCurrentFileStructure(filename);
					if (currentFD != -1) //file opened already
					{
						ret = -1;
					}
					else {

						//check mode in case not owner
						if (inodes[nodeIdx].ownerTaskID != pTCB->id)
						{
							ret = -1;// not this task
						}
						else {

							//clear data 
							//check at index = 15, ... 0
							for (int i = fsNumBLocks - 1; i >= 0; i--)
							{
								if (GetBit(inodes[nodeIdx].blocks, i))
								{
									for (size_t j = 0; j < fsBlockSize; j++)
									{
										writeCharToFile(DATA_FILE, i * fsBlockSize + j, initializationChar);
									}
								}
							}

							//delete inode
							memset((void*)&inodes[nodeIdx], 0, sizeof(Inode));
						}
					}
				}

				////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

//check if the bit at kth is 1
bool ufs::GetBit(unsigned int blocks, unsigned int k) {
	return ((blocks & (0x01 << k)) != 0);
}

//change permission
//only owner can change permission
int ufs::changePermission(TCB* pTCB, const char* filename, const char* newPermission) {

	int numResources = 0;

	int ret = 0; //return value

	//the task need 1 resource
	while (numResources < 1 && ret != -1)
	{
		if (pTCB->state == RUNNING)
		{
			if (resource->down(pTCB)) {//request resource is succesful

				while (pTCB->state != READY && pTCB->state != RUNNING) {
					sleep(1);
				}

				/////////////critcal section

				//find inode
				int nodeIdx = findInodeIndex(filename);

				if (nodeIdx == -1) //file not found
				{
					ret = -1;
				}
				else {

					//get index of fileHandles by file name
					int currentFD = getCurrentFileStructure(filename);
					if (currentFD != -1) //file opened already
					{
						ret = -1;
					}
					else {

						//check mode in case not owner
						if (inodes[nodeIdx].ownerTaskID != pTCB->id)
						{
							ret = -1; //on owner can change permission
						}
						else {

							//change permission
							strcpy(inodes[nodeIdx].permission, newPermission);

							time(&inodes[nodeIdx].lastModifiedTime);
						}
					}
				}

				////////////end critcal section

				//release the resource
				resource->up(pTCB);
				numResources++;
			}
		}
	}

	return ret;
}

void ufs::dir(char largeBuffer[]) { //show directory

	stringstream ss;
	ss << endl;
	ss << " File     File      Blocks used Size  Starting  Status     Permission Owner Create                   Modified  " << endl;
	ss << " Handle   Name                        Block                                 Time                     Time      " << endl;

	for (int i = 0; i < fsNumBLocks; i++)
	{
		if (strlen(inodes[i].filename) > 0)
		{
			ss << " ";

			//get index of fileHandles by file handle
			int idx = getCurrentFileStructure(inodes[i].filename);

			if (idx == -1)
			{
				ss << left << setw(9) << "";
			}
			else {
				ss << left << setw(9) << fileHandles[idx].fileHandle;
			}
			ss << left << setw(10) << inodes[i].filename;

			//calculate number of blocks to store file
			int numBlocks = inodes[i].size / fsBlockSize;
			if (inodes[i].size % fsBlockSize != 0)
			{
				numBlocks++;
			}

			//blocks are contiguous
			for (size_t j = 0; j < 4; j++)
			{
				if (j < numBlocks)
				{
					ss << left << setw(3) << inodes[i].startingBlock + j;
				}
				else {
					ss << left << setw(3) << "";
				}
			}
			ss << left << setw(6) << inodes[i].size;

			ss << left << setw(10) << inodes[i].startingBlock;

			if (idx == -1)
			{
				ss << left << setw(11) << "closed";
			}
			else {
				if (strcmp(fileHandles[idx].mode, "r") == 0)
				{
					ss << left << setw(11) << "open_read";
				}
				else if (strcmp(fileHandles[idx].mode, "w") == 0)
				{
					ss << left << setw(11) << "open_write";
				}
			}
			ss << left << setw(11) << inodes[i].permission;
			ss << left << setw(6) << inodes[i].ownerTaskID;

			struct tm * timeinfo;
			char bufTime[512];

			timeinfo = localtime(&inodes[i].creationTime);
			strftime(bufTime, 512, "%c", timeinfo);

			ss << left << setw(25) << bufTime;

			timeinfo = localtime(&inodes[i].lastModifiedTime);
			strftime(bufTime, 512, "%c", timeinfo);

			ss << left << setw(25) << bufTime;
			ss << endl;
		}
	}

	//copy to buffer
	strcpy(largeBuffer, ss.str().c_str());
}

void ufs::dump(char largeBuffer[]) {    //dump file data

	fstream afile(DATA_FILE, ios::in); //open file to read

	int i = 0;

	stringstream ss;
	ss << endl;
	ss << " ";
	while (i < fsNumBLocks * fsBlockSize)
	{
		char ch;
		afile >> ch;

		ss << ch;

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

	afile.close(); //close file
}

//write to file
void ufs::writeCharToFile(const char* filename, int pos, char ch) {
	
	fstream afile(filename, ios::in|ios::out); //open file to write

	afile.seekp(pos); // Place the file pointer
	afile << ch;

	afile.close(); //close file
}

//read from file
void ufs::readCharFromFile(const char* filename, int pos, char* ch) {

	fstream afile(filename, ios::in); //open file to read

	afile.seekp(pos); // Place the file pointer
	afile >> *ch;

	afile.close(); //close file

}
