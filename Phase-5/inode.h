#ifndef I_NODE_H
#define I_NODE_H

#include <time.h>

//i-node class
class Inode {
public:
	char filename[8];  //file name: max 7 bytes + null byte
	int ownerTaskID;   //task ID
	int startingBlock; //starting block
	int size;          //file size in bytes
	char permission[5];//file permission such as rwrw

					   /*each file can in 4 blocks
					   use the bitmap so blocks with 16 bits (with 16 blocks) is used only
					   */
	unsigned int blocks; //4 bytes

	int x; //4 bytes - unused

	time_t creationTime;     // 4 bytes
	time_t lastModifiedTime; // 4 bytes
};


#endif
