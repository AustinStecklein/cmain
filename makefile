CC = gcc
CC_FLAGS = -Wall
LD_FLAGS = -lm
DEBUG = -ggdb3

clean:
	rm *.gch
	rm *.o
	rm ./build/*

debug_arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -DARENA_DEBUG -c arena.h arena.c

arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -c arena.h arena.c

arena: debug_arena.o
	$(CC) $(LD_FLAGS) arena.o -o ./build/arena

array.o: array.c array.h arena.h arena.c
	$(CC) $(CC_FLAGS) -c array.h array.c

array: arena.o array.o
	$(CC) $(LD_FLAGS) array.o arena.o -o ./build/array

unittest.o: unittest.c array.h arena.h arena.c
	$(CC) $(CC_FLAGS) $(DEBUG) -c unittest.c

unittest: unittest.o arena.o
	$(CC) $(LD_FLAGS) arena.o unittest.o -o ./build/unittest
