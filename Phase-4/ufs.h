#ifndef UFS_H
#define UFS_H

#include "inode.h"
#include "Sema.h"

#include <fstream>       // To be able to use File I/O
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

//number of files that are currently being opened
#define MAX_OPENED_FILES 100

#define DATA_FILE "data"
#define INODE_FILE "inode"

//current file that is opened
struct CurrentFile {

	int fileHandle;  //file handle
	int taskID;   //task that open this file
	int currentLocation; //current location to read or write
	int nodeID;  //id of node
	char mode[2]; //mode: r or w
};

/*ufs class*/
class ufs
{
public:
	string fsName;     //name of FS
	int fsBlockSize;   //size of one block
	int fsNumBLocks;   //number of blocks
	int nextFileHandle;//ufsCreateFile gets the next handle
	char initializationChar; //initialization character

	//file system opeations
	//constructor
	ufs(string name, int noBlocks, int blockSize, char initChar);

	~ufs(); //destructor

	void format(); //format the current file system

	//file operations
	//open file, return -1 if fail; otherwise, return file handle
	int open(TCB* pTCB, const char* filename, const char* mode);

	//close file
	int close(TCB* pTCB, int fileHandle);

	//read char, keep track current file location
	int readChar(TCB* pTCB, int fileHandle, char* charContent);

	//write a char
	int writeChar(TCB* pTCB, int fileHandle, char charContent);

	//directory operations
	int createFile(TCB* pTCB, const char* filename, int fileSize, const char* permission);

	//delete file
	int deleteFile(TCB* pTCB, const char* filename);

	//change permission
	//only owner can change permission
	int changePermission(TCB* pTCB, const char* filename, const char* newPermission);

	void dir(char largeBuffer[]); //show directory
	void dump(char largeBuffer[]);    //dump file data

private:

	//e.g 16 inodes
	Inode* inodes;

	//data in blocks, e.g 16 blocks, 128 bytes each block
	//char* data;
	
	void writeCharToFile(const char* filename, int pos, char ch);
	void readCharFromFile(const char* filename, int pos, char* ch);

	//find free inode
	int findFreeInode();

	//find contiguous file blocks
	int findContigousFileBlocks(int numRequiredBlocks);

	//generate the bitmap that uses the freed blocks
	//such as 1111 0000 0000 0000 ->got 4 freed blocks then put in i-node
	int acquireBlocks(int startingBit, int numRequiredBlocks);

	//find inode by file name
	int findInodeIndex(const char* filename);

	//the current opened files
	CurrentFile fileHandles[MAX_OPENED_FILES];

	//get index of fileHandles by file handle
	int getCurrentFileStructureByFD(int fileHandle);

	//get freed CurrentFile structure in array
	int getFreedFileStructure();

	//get index of fileHandles by file name
	int getCurrentFileStructure(const char* filename);

	//check if the bit at kth is 1
	bool GetBit(unsigned int blocks, unsigned int k);

	//create semaphore to protect resource (inode file)
	Semaphore* resource;
};

#endif
