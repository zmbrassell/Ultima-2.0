#include "mcb.h"

//constructor
MCB::MCB() {
	Monitor = new Semaphore("Monitor", 1);
	Printer = new Semaphore("Printer", 1);
	memoryManager = new MemMgr(1024, '.');
	fileSystem = new ufs("root", 16, 128, '^');
}

//destructor
MCB::~MCB() {
	delete Monitor;
	delete Printer;
	delete memoryManager;
	delete fileSystem;

	//cout << "~MCB was called" << endl;
}