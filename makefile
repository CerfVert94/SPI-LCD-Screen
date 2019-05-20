CC = gcc
CFLAGS = -Wall 
LDFLAGS = -lm -lwiringPi
EXEC = main

DEBUGGER = gdb
DEBUG_EXEC = main_debug
DEPEND_SRC = $(wildcard Sys/*.c)
DEPEND_OBJ = $(wildcard Sys/*.o)
DEBUG_FLAG = -DNBDEBUG=1
all: depend exec

exec: 
	$(CC) -o $(EXEC) main.c $(DEPEND_OBJ) $(CFLAGS) $(LDFLAGS)
init:
	$(CC) -o $@ init.c $(DEPEND_OBJ) $(CFLAGS)

depend: gpio timers

Sys/%.o : Sys/%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@

gpio : Sys/gpio.o
	
timers: Sys/timers.o

debug_depend : CFLAGS += -DNDEBUG
debug_depend : depend

tests:
	$(CC) -o timertest UnitTest/timertest.c $(DEPEND_OBJ) $(CFLAGS) 

clean: 
	rm $(wildcard *.o) $(DEPEND_OBJ) $(EXEC) init
