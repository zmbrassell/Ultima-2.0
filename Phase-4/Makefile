CCC = g++
CCLINKER = $(CCC)
INCLUDE_DIR =
CFLAGS = $(INCLUDE_DIR)
CCFLAGS = $(CFLAGS)
OPTFLAGS = -g
LIBS_PATH =
LIB_LIST = -lm -lpthread -lncurses
LIBS = $(LIB_PATH) $(LIB_LIST)

EXEC = a.out

objects = Ultima.o Sema.o Sched.o ipc.o mcb.o MemMgr.o ufs.o

default = $(EXEC)

$(EXEC): $(objects)
	g++ -g -o $(EXEC) $(objects) $(LIBS)


Sema.o: TCB.h Queue.h Sema.h Sema.cpp Sched.h
	g++ -c Sema.cpp -o Sema.o


Sched.o: TCB.h Sched.h Sched.cpp
	g++ -c Sched.cpp -o Sched.o

mcb.o: mcb.h mcb.h mcb.cpp
	g++ -c mcb.cpp -o mcb.o

ipc.o: ipc.h ipc.h ipc.cpp
	g++ -c ipc.cpp -o ipc.o

MemMgr.o: TCB.h MemMgr.h MemMgr.cpp
	g++ -c MemMgr.cpp -o MemMgr.o

ufs.o: inode.h ufs.h ufs.cpp
	g++ -c ufs.cpp -o ufs.o

Ultima.o: Ultima.h Ultima.cpp Sched.h Sema.h ufs.h
	g++  -c Ultima.cpp -o Ultima.o 

clean:
	rm *.o
	rm $(EXEC)
