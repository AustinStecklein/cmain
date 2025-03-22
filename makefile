CC = gcc
CC_FLAGS = -Wall
LD_FLAGS = -lm
DEBUG = -ggdb3

debug_arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -DARENA_DEBUG -c arena.c

arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -c arena.c

arena: debug_arena.o
	$(CC) $(LD_FLAGS) arena.o -o ./build/arena

array.o: array.c array.h arena.h arena.c unittest.h
	$(CC) $(CC_FLAGS) -c array.c

array: arena.o array.o unittest.h
	$(CC) $(LD_FLAGS) array.o arena.o -o ./build/array

unittest.o: unittest.c unittest.h array.h arena.h arena.c
	$(CC) $(CC_FLAGS) $(DEBUG) -c unittest.c

unittest: unittest.o arena.o
	$(CC) $(LD_FLAGS) arena.o unittest.o -o ./build/unittest

string.o: string.c array.h arena.h string.h
	$(CC) $(CC_FLAGS) $(DEBUG) -c string.c

string: string.o arena.o
	$(CC) $(LD_FLAGS) arena.o string.o -o ./build/string

.PHONY: clean
clean:
	rm *.o
	rm ./build/*
	rm *.gch


