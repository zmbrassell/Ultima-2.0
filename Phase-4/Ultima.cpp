#include "Ultima.h"
#include "mcb.h"

// ULTIMA FILE FOR PHASE 4

using namespace std;

//number of threads to test
#define NUM_THREADS 3

//number of message types
#define NUM_MESSAGE_TYPES 3

/*Message type*/
MessageType messageTypes[NUM_MESSAGE_TYPES];

//master control block
MCB masterControlBlock;

//array of TCB*
TCB* TCBArray[NUM_THREADS];

bool Paused = false; // for pausing the OS

//*******************************************************************************
struct thread_data {
	int thread_no;
	WINDOW* thread_win;
	bool kill_signal;
	int sleep_time;
	int thread_results;

	TCB* pTCP;

};
//********************************************************************************

//----------------------------------------------------------------- 
//  Global Mutual Exclusion // 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//----------------------------------------------------------------- 

//*******************************************************************************
// Forward declaration --> was moved to Ultima.h
//*******************************************************************************
//WINDOW* create_window(int height, int width, int starty, int startx);
//void write_window(WINDOW* Win, const char* text);
//void write_window(WINDOW* Win, int x, int y, const char* text);
//*******************************************************************************

//task run by pthread
//in this version of fileSystemTestTask we don't delete the files at the end
void* fileSystemTestTask1(void* arguments)
{
	char* ret; // Return value
	char filename[10];

	// extract the thread arguments and cast arguments into thread_data  
	thread_data* td = (thread_data*)arguments;
	int thread_no = td->thread_no;
	int sleep_time = td->sleep_time;
	WINDOW* Win = td->thread_win;
	TCB* pTCP = td->pTCP;
	pTCP->taskWin = Win;

	int memHandle = -1;

	int numSuccess = 0; //run some times

	char randomString[1024]; //random string
	char buff[256];       //buffer to read from string
	char largeBuffer[1024];
	sprintf(buff, " %s ready \n", pTCP->name);
	write_window(Win, buff);

	while (numSuccess < 5)
	{
		if (pTCP->state == RUNNING && !Paused) {

			if (pTCP->id == 0)//task 0
			{
				//create file with no other permission
				masterControlBlock.fileSystem->createFile(pTCP, "filex", 128, "rw__");

				sprintf(buff, " created file with no other permission\n");
				write_window(Win, buff);
				numSuccess++;
			}

			//Create a file
			sprintf(filename, "file %d", (pTCP->id * 3 + 1));
			if (masterControlBlock.fileSystem->createFile(pTCP, filename, rand() % 412 + 100, "rwrw") == 0) {

				sprintf(buff, " created file: %s\n", filename);
				write_window(Win, buff);
				numSuccess++;
				//open file
				int fd = masterControlBlock.fileSystem->open(pTCP, filename, "w");
				if (fd != -1)
				{
					//write some characters
					for (size_t i = 0; i < 5; i++)
					{
						char c = (char)('a' + rand() % 26);
						masterControlBlock.fileSystem->writeChar(pTCP, fd, c);

						sprintf(buff, " write: %c \n", c);
						write_window(Win, buff);
					}
					numSuccess++;

					sprintf(buff, " \n");
					write_window(Win, buff);

					//close file
					masterControlBlock.fileSystem->close(pTCP, fd);

					sprintf(buff, " close file: %s. Open again for reading\n", filename);
					write_window(Win, buff);
					numSuccess++;

					fd = masterControlBlock.fileSystem->open(pTCP, filename, "r");

					for (size_t i = 0; i < 5; i++) {
						//read
						char c;
						masterControlBlock.fileSystem->readChar(pTCP, fd, &c);

						sprintf(buff, " read: %c \n", c);
						write_window(Win, buff);
						numSuccess++;
					}

					sprintf(buff, " \n");
					write_window(Win, buff);

					masterControlBlock.fileSystem->close(pTCP, fd);

					//close file
					sprintf(buff, " close file: %s\n", filename);
					write_window(Win, buff);
					numSuccess++;
				}

				sleep(1);

				//delete file
				/*if (pTCP->id != 0) {
					sprintf(buff, " delete file: %s\n", filename);
					masterControlBlock.fileSystem->deleteFile(pTCP, filename);
					write_window(Win, buff);
					numSuccess++;
				}*/

			}
			else {
				sprintf(buff, " could not create file: %s\n", filename);
				write_window(Win, buff);
			}

			sleep(1);

			//create file too much size
			if (masterControlBlock.fileSystem->createFile(pTCP, "bigfile", 999, "rwrw") == 0) {
				sprintf(buff, "SOMETHING IS WRONG. WAS ABLE TO CREATE A 2 BIG FILE\n");
				write_window(Win, buff);
			}
			else {
				sprintf(buff, " TEST SUCCESS: could not create too big size\n");
				write_window(Win, buff);
				numSuccess;
			}

			if (pTCP->id == 1)//task 1
			{
				int hd2 = masterControlBlock.fileSystem->open(pTCP, "filex", "r");
				if (hd2 == -1)
				{
					sprintf(buff, " TEST SUCCESS: no permission to open file to read\n\n");
					write_window(Win, buff);
					numSuccess;
				}



			}

			if (pTCP->id == 2)//task 2
			{
				int hd2 = masterControlBlock.fileSystem->open(pTCP, "filex", "w");
				if (hd2 == -1)
				{
					sprintf(buff, " TEST SUCCESS: no permission to open file to write\n\n");
					write_window(Win, buff);
					numSuccess++;
				}
			}
		}
	}
	pTCP->state = DEAD;
	sprintf(buff, " TASK COMPLETE\n");
	write_window(Win, buff);

	pthread_exit(ret);
}



void* fileSystemTestTask2(void* arguments)
{
	char* ret; // Return value
	char filename[10];

	// extract the thread arguments and cast arguments into thread_data  
	thread_data* td = (thread_data*)arguments;
	int thread_no = td->thread_no;
	int sleep_time = td->sleep_time;
	WINDOW* Win = td->thread_win;
	TCB* pTCP = td->pTCP;
	pTCP->taskWin = Win;

	int memHandle = -1;

	int numSuccess = 0; //run some times

	char randomString[1024]; //random string
	char buff[256];       //buffer to read from string
	char largeBuffer[1024];
	sprintf(buff, " %s ready \n", pTCP->name);
	write_window(Win, buff);

	while (numSuccess < 5)
	{
		if (pTCP->state == RUNNING && !Paused) {

			if (pTCP->id == 0)//task 0
			{
				//create file with no other permission
				masterControlBlock.fileSystem->createFile(pTCP, "filex", 128, "rw__");

				sprintf(buff, " created file with no other permission\n");
				write_window(Win, buff);
				numSuccess++;
			}

			//Create a file
			sprintf(filename, "file %d", (pTCP->id * 3 + 1));
			if (masterControlBlock.fileSystem->createFile(pTCP, filename, rand() % 412 + 100, "rwrw") == 0) {

				sprintf(buff, " created file: %s\n", filename);
				write_window(Win, buff);
				numSuccess++;
				//open file
				int fd = masterControlBlock.fileSystem->open(pTCP, filename, "w");
				if (fd != -1)
				{
					//write some characters
					for (size_t i = 0; i < 5; i++)
					{
						char c = (char)('a' + rand() % 26);
						masterControlBlock.fileSystem->writeChar(pTCP, fd, c);

						sprintf(buff, " write: %c \n", c);
						write_window(Win, buff);
					}
					numSuccess++;

					sprintf(buff, " \n");
					write_window(Win, buff);

					//close file
					masterControlBlock.fileSystem->close(pTCP, fd);

					sprintf(buff, " close file: %s. Open again for reading\n", filename);
					write_window(Win, buff);
					numSuccess++;

					fd = masterControlBlock.fileSystem->open(pTCP, filename, "r");

					for (size_t i = 0; i < 5; i++) {
						//read
						char c;
						masterControlBlock.fileSystem->readChar(pTCP, fd, &c);

						sprintf(buff, " read: %c \n", c);
						write_window(Win, buff);
						numSuccess++;
					}

					sprintf(buff, " \n");
					write_window(Win, buff);

					masterControlBlock.fileSystem->close(pTCP, fd);

					//close file
					sprintf(buff, " close file: %s\n", filename);
					write_window(Win, buff);
					numSuccess++;
				}

				sleep(1);

				//delete file
				if (pTCP->id != 0) {
					sprintf(buff, " delete file: %s\n", filename);
					masterControlBlock.fileSystem->deleteFile(pTCP, filename);
					write_window(Win, buff);
					numSuccess++;
				}

			}
			else {
				sprintf(buff, " could not create file: %s\n", filename);
				write_window(Win, buff);
			}

			sleep(1);

			//create file too much size
			if (masterControlBlock.fileSystem->createFile(pTCP, "bigfile", 999, "rwrw") == 0) {
				sprintf(buff, "SOMETHING IS WRONG. WAS ABLE TO CREATE A 2 BIG FILE\n");
				write_window(Win, buff);
			}
			else {
				sprintf(buff, " TEST SUCCESS: could not create too big size\n");
				write_window(Win, buff);
				numSuccess;
			}

			if (pTCP->id == 1)//task 1
			{
				int hd2 = masterControlBlock.fileSystem->open(pTCP, "filex", "r");
				if (hd2 == -1)
				{
					sprintf(buff, " TEST SUCCESS: no permission to open file to read\n\n");
					write_window(Win, buff);
					numSuccess;
				}



			}

			if (pTCP->id == 2)//task 2
			{
				int hd2 = masterControlBlock.fileSystem->open(pTCP, "filex", "w");
				if (hd2 == -1)
				{
					sprintf(buff, " TEST SUCCESS: no permission to open file to write\n\n");
					write_window(Win, buff);
					numSuccess++;
				}
			}
		}
	}
	pTCP->state = DEAD;
	sprintf(buff, " TASK COMPLETE\n");
	write_window(Win, buff);

	pthread_exit(ret);
}




//the thread that run to write the dump message
void* logThreadTask(void * arguments) {

	char buff[1024];
	char largeBuffer[100 * 1024];

	// extract the thread arguments and cast arguments into thread_data   
	thread_data* td = (thread_data*)arguments;
	int thread_no = td->thread_no;
	int sleep_time = td->sleep_time;
	WINDOW* Win = td->thread_win;

	sleep(2);
	wclear(Win);
	sprintf(buff, " \n FILESYSTEM DUMP BEFORE THE TASKS START\n");
	write_window(Win, buff);
	sleep(2);
	// Filesystem Dump
	bool dumped = false;
	while (!dumped) {
		if (!Paused) {
			masterControlBlock.fileSystem->dump(largeBuffer);
			write_window(Win, largeBuffer);
			sleep(2);
			dumped = true;
		}
	}

	// Directory Dump
	dumped = false;
	while (!dumped) {
		if (!Paused) {
			masterControlBlock.fileSystem->dir(largeBuffer);
			write_window(Win, largeBuffer);
			sleep(2);
			dumped = true;
		}
	}
	
	//Keep running the loop while there is still at least one thread alive
	bool allDead = false;
	while (!allDead)
	{
		if (!Paused) {
			//Clear the window
			wclear(Win);

			//Attempt to yield
			bool yielded = false;
			while (!yielded) {
			if (!Paused) {
				masterControlBlock.Swapper.yield();
				yielded = true;
			}
		   }
			//***Dump for testing purposes***
			bool dumped = false;
			//Scheduler Dump and clear window
			while (!dumped) {
				if (!Paused) {
					masterControlBlock.Swapper.dump(0, largeBuffer);
					write_window(Win, largeBuffer);
					sleep(2);
					wclear(Win);
					dumped = true;
				}
			}

			// Filesystem Dump
			dumped = false;
			while (!dumped) {
				if (!Paused) {
					masterControlBlock.fileSystem->dump(largeBuffer);
					write_window(Win, largeBuffer);
					sleep(2);
					dumped = true;
				}
			}

			// Directory Dump
			dumped = false;
			while (!dumped) {
				if (!Paused) {
					masterControlBlock.fileSystem->dir(largeBuffer);
					write_window(Win, largeBuffer);
					sleep(2);
					dumped = true;
				}
			}


			// Create an iterator to go through the process table with
			TCB* current = masterControlBlock.Swapper.processTable;

			// Iterate through the table to see if any threads are alive
			bool atLeastOneAlive = false;
			for (size_t i = 0; i < NUM_THREADS; i++)
			{
				if (current->state != DEAD)
				{
					// living thread has been found exit loop
					atLeastOneAlive = true;
					break;
				}
				current = current->next;
			}


			//All threads are dead, time to exit the main loop
			if (!atLeastOneAlive)
			{
				//Dump for testing purposes


				allDead = true; //cause the exit condition for the main loop

			// Scheduler Dump --> everything should be dead
				dumped = false; 
				while (!dumped) {
					if (!Paused) {
						masterControlBlock.Swapper.dump(0, largeBuffer);
						write_window(Win, largeBuffer);
						sleep(3);
						wclear(Win);
						dumped = true;
					}
				}
				// Filesystem Dump
				dumped = false;
				while (!dumped) {
					if (!Paused) {
						masterControlBlock.fileSystem->dump(largeBuffer);
						write_window(Win, largeBuffer);
						sleep(2);
						dumped = true;
					}
				}

				// Directory Dump
				dumped = false;
				while (!dumped) {
					if (!Paused) {
						masterControlBlock.fileSystem->dir(largeBuffer);
						write_window(Win, largeBuffer);
						sleep(2);
						dumped = true;
					}
				}

				sprintf(buff, " ........Everything has died.....\n");
				write_window(Win, buff);
				break;
			}
		}
	}
	
}

/*
main function to start C++ application
*/
int main() {

	//set up some message types
	messageTypes[0].MessageTypeId = 0;
	strcpy(messageTypes[0].MessageTypeDescription, "Text");

	messageTypes[1].MessageTypeId = 1;
	strcpy(messageTypes[1].MessageTypeDescription, "Service");

	messageTypes[2].MessageTypeId = 2;
	strcpy(messageTypes[2].MessageTypeDescription, "Notification");

	//create the area of threads 
	pthread_t pThreads[NUM_THREADS];
	thread_data thread_args[NUM_THREADS];

	thread_data dumpthread_arg; //for dump

	//for dump message to check
	pthread_t pDumpThread;

	//initialize time
	srand(time(0));

	//create TCB
	for (size_t i = 0; i < NUM_THREADS; i++)
	{
		string name = "Task";
		name += (char)(i + '0');

		//create the task
		TCBArray[i] = CreateTask(&masterControlBlock.Swapper, name.c_str());
	}

	//*****************************************************************************
	initscr();   // Start nCurses

	refresh();

	//*****************************************************************************
	// create a window to display thread data in
	// Create a new window : WINDOW * win = newwin(nlines, ncols, y0, x0;

	pthread_mutex_lock(&mutex);

	WINDOW* Heading_Win = newwin(12, 180, 3, 2);
	box(Heading_Win, 0, 0);
	mvwprintw(Heading_Win, 2, 28, "ULTIMA 2.0");

	mvwprintw(Heading_Win, 4, 2, "Starting Ultima 2.0.......");
	mvwprintw(Heading_Win, 5, 2, "Starting Thread 1.....");
	mvwprintw(Heading_Win, 6, 2, "Starting Thread 2.....");
	mvwprintw(Heading_Win, 7, 2, "Starting Thread 3.....");
	mvwprintw(Heading_Win, 9, 2, "Press 'q' or Ctrl-C to exit the program...");
	wrefresh(Heading_Win); //changes to the window are refreshed

	pthread_mutex_unlock(&mutex);

	//****************************************************************************
	WINDOW* Log_Win = create_window(20, 160, 30, 2);
	write_window(Log_Win, 1, 1, "......Log.....\n");

	//***************************************************************************
	WINDOW* Console_Win = create_window(20, 20, 30, 162);
	write_window(Console_Win, 1, 1, "....Console....\n");
	write_window(Console_Win, 2, 1, "Ultima # ");

	write_window(Log_Win, ".......Main program started.........\n");

	//***************************************************************************

	int result_code;

	write_window(Log_Win, " Creating thread 1\n");
	// set the thread args
	thread_args[0].thread_no = 1;
	thread_args[0].thread_win = create_window(15, 60, 15, 2);
	write_window(thread_args[0].thread_win, 6, 1, "Starting Thread 1.....\n");
	thread_args[0].sleep_time = 1 + rand() % 3; // get an integer between 1 and 3
	thread_args[0].kill_signal = false;
	thread_args[0].thread_results = 0; // initialize results
	thread_args[0].pTCP = TCBArray[0]; // TCB
	result_code = pthread_create(&pThreads[0], NULL, fileSystemTestTask1, &thread_args[0]);
	assert(!result_code); // if there is any problems with result code. display it and end program.


	//****************************************************************************

	write_window(Log_Win, " Creating thread 2\n");
	// set the thread_args
	thread_args[1].thread_no = 2;
	thread_args[1].thread_win = create_window(15, 60, 15, 62);
	write_window(thread_args[1].thread_win, 6, 1, "Starting Thread 2.....\n");
	thread_args[1].sleep_time = 1 + rand() % 3; // get an integer between 1 and 3
	thread_args[1].kill_signal = false;
	thread_args[1].thread_results = 0;
	thread_args[1].pTCP = TCBArray[1]; // TCB
	result_code = pthread_create(&pThreads[1], NULL, fileSystemTestTask1, &thread_args[1]);
	assert(!result_code); // if there is any problems with result code. dispay it and end program.

	 //*****************************************************************************

	write_window(Log_Win, " Creating thread 3\n");
	// set the thread_args
	thread_args[2].thread_no = 3;
	thread_args[2].thread_win = create_window(15, 60, 15, 122);
	write_window(thread_args[2].thread_win, 6, 1, "Starting Thread 3.....\n");
	thread_args[2].kill_signal = false;
	thread_args[2].sleep_time = 1 + rand() % 3; // get an integer between 1 and 3
	thread_args[2].thread_results = 0; // initialize results
	thread_args[2].pTCP = TCBArray[2]; // TCB
	result_code = pthread_create(&pThreads[2], NULL, fileSystemTestTask1, &thread_args[2]);
	assert(!result_code); // if there is any probles with result code. display it and end program
	

	/******** create thread for dump message****/
	// set the thread args
	dumpthread_arg.thread_no = 4;
	dumpthread_arg.thread_win = Log_Win;
	dumpthread_arg.sleep_time = 1 + rand() % 3; // get an integer between 1 and 3
	dumpthread_arg.kill_signal = false;
	dumpthread_arg.thread_results = 0; // initialize results

	result_code = pthread_create(&pDumpThread, NULL, logThreadTask, &dumpthread_arg);
	assert(!result_code); // if there is any problems with result code. dispay it and end program.

	// ******************************************************************************
	write_window(Log_Win, " All threads have been created... \n");	

	//*********************************************************************************
	// Set up keyboard I/O processing
	cbreak(); // disable line buffering
	noecho(); // disable automatic echo of characters read b getch(), wgetch()
	nodelay(Console_Win, true); // nodelay causes getch to be a non blockking call.
								// if no input is ready, get ch returns ERR

	int input = -1;
	char buff[256];

	while (input != 'q')
	{
		input = wgetch(Console_Win);

		switch (input)
		{

		case 'c':
			// clear the console window
			refresh(); // clear the entire screen (in case it is corrupted)
			wclear(Console_Win); // clear the console window
			write_window(Console_Win, 1, 1, "Ultima # ");
			break;

		case 'q':
			// end the loop and end the proram.
			write_window(Log_Win, " Quitting the main program....\n");
			break;
		case 'p':
			// pause/unpause the execution
			if (Paused) {
				Paused = false;
				write_window(Console_Win, "\n UNPAUSED\n");
			}
			else {
				Paused = true;
				write_window(Console_Win, "\n PAUSED\n");
			}
			break;
		case ERR:
			//if wgetch() return ERR, that means no keys were pressed
			// earlier we enabled non-blocking input using nodelay() see above
			// this allows the program to continue to inspect the keynoard without
			// having to wait fo the key to be pressed.
			break;
		default:
			sprintf(buff, " %c\n", input);
			write_window(Console_Win, buff);
			write_window(Console_Win, " -Invalid Command\n");
			write_window(Log_Win, buff);
			write_window(Log_Win, " -Invalid Command\n");
			write_window(Console_Win, "Ultima # ");
			break;
		}
		sleep(1);
	}

	write_window(Log_Win, " Waiting for living threads to complete..\n");

	//wait for thread then close the program
	for (size_t i = 0; i < NUM_THREADS; i++)
	{
		result_code = pthread_join(pThreads[i], NULL);
	}

	result_code = pthread_join(pDumpThread, NULL); //log thread

	write_window(Log_Win, " All threads have now ended......\n");
	write_window(Log_Win, "......Main program ended......\n");

	getch();
	endwin(); // End the curses window

	exit(0);

	return 0;
}

// create task return the task ID
TCB* CreateTask(Scheduler* scheduler, const char *taskname) {
	return scheduler->createTask(taskname);
}

//generate random string
void GenRandomString(char *s, const int len){
	static const char alphanum[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len - 1; i++) {
        s[i] = alphanum[rand() % strlen(alphanum)];
    }

    s[len] = 0;
}

//---------------------------------------------------------------- 
void identify_window(void) {
	int Y, X;
	int Max_X, Max_Y;
	getmaxyx(stdscr, Max_Y, Max_X); // get screen size   
	wprintw(stdscr, "Screen Height = %d, Screen Width  = %d\n", Max_Y, Max_X);
	getyx(stdscr, Y, X); // get current y,x coordinate   
	wprintw(stdscr, "Current Y = %d , Current X = %d\n", Y, X);
}

//***************************************************************************
WINDOW* create_window(int height, int width, int starty, int startx)
{
	WINDOW* Win;
	pthread_mutex_lock(&mutex);
	// ---- Critical Region ----- 
	Win = newwin(height, width, starty, startx);
	scrollok(Win, TRUE); // allow scroling of the window
	scroll(Win); // scroll the window
	box(Win, 0, 0); // 0, 0 gives default characters
					// for the vertical and horizontal ines
	wrefresh(Win);  // draw the window
					// ---- Critical Region -----    
	pthread_mutex_unlock(&mutex);
	return Win;
}
//**************************************************************************
void write_window(WINDOW* Win, const char* text)
{
	pthread_mutex_lock(&mutex);
	// ---- Critical Region ----- 
	wprintw(Win, text);
	box(Win, 0, 0);
	wrefresh(Win); // draw the window
	pthread_mutex_unlock(&mutex);
}
//*************************************************************************
void write_window(WINDOW* Win, int x, int y, const char* text)
{
	pthread_mutex_lock(&mutex);
	// ---- Critical Region ----- 
	mvwprintw(Win, y, x, text);
	box(Win, 0, 0);
	wrefresh(Win); // draw the window
	pthread_mutex_unlock(&mutex);
}
