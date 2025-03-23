CC = gcc
CC_FLAGS = -Wall
LD_FLAGS = -lm
DEBUG = -ggdb3

debug_arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -DARENA_DEBUG -c arena.c -o ./build/debug_arena.o

debug_arena_val.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -DARENA_DEBUG -DVALGRIND -c arena.c -o ./build/debug_arena_val.o

arena.o: arena.h arena.c
	$(CC) $(CC_FLAGS) -c arena.c -o ./build/arena.o

arena: debug_arena.o
	$(CC) $(LD_FLAGS) ./build/debug_arena.o -o ./build/arena

arena_val: debug_arena_val.o
	$(CC) $(LD_FLAGS) ./build/debug_arena_val.o -o ./build/arena_val
	valgrind --leak-check=full ./build/arena_val

array.o: array.c array.h arena.h arena.c unittest.h
	$(CC) $(CC_FLAGS) -c array.c -o ./build/array.o

array: arena.o array.o unittest.h
	$(CC) $(LD_FLAGS) ./build/array.o ./build/arena.o -o ./build/array

unittest.o: unittest.c unittest.h array.h arena.h arena.c
	$(CC) $(CC_FLAGS) $(DEBUG) -c unittest.c -o ./build/unittest.o

unittest: unittest.o arena.o
	$(CC) $(LD_FLAGS) ./build/arena.o ./build/unittest.o -o ./build/unittest

string.o: string.c array.h arena.h string.h
	$(CC) $(CC_FLAGS) $(DEBUG) -c string.c -o ./build/string.o

string: string.o arena.o
	$(CC) $(LD_FLAGS) ./build/arena.o ./build/string.o -o ./build/string


.PHONY: clean
clean:
	rm *.o
	rm ./build/*
	rm *.gch


