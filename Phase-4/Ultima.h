#ifndef ULTIMA_H
#define ULTIMA_H

#include "Sema.h"
#include "Sched.h"
#include <pthread.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <sstream>
#include <ncurses.h>

// create return the task ID
TCB* CreateTask(Scheduler* scheduler, const char *taskname);

//generate random string
void GenRandomString(char *s, const int len);

WINDOW * create_window(int height, int width, int starty, int startx);

void write_window(WINDOW* Win, const char* text);

void write_window(WINDOW* Win, int x, int y, const char* text);


#endif
