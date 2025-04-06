.DELETE_ON_ERROR:
CC = gcc
CC_FLAGS = -Wall -MMD -MP
LD_FLAGS = -lm
DEBUG = -ggdb3
ASM = nasm
ASM_FLAGS = -felf64 -g
LD = ld

BUILD_DIR := build

SRCS := $(shell find . -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
ARENA_DEPS := $(BUILD_DIR)/arena.o
ARRAY_DEPS := $(BUILD_DIR)/arena.o
ARRAY_DEPS += $(BUILD_DIR)/array.o

UNITTEST_DEPS := $(BUILD_DIR)/unittest.o
UNITTEST_DEPS += $(BUILD_DIR)/arena.o
STRING_DEPS := $(BUILD_DIR)/string.o
STRING_DEPS += $(BUILD_DIR)/arena.o

# build allocator for valgrind testing
arena_mem: DEBUG += -DVALGRIND
arena_mem: DEBUG += -DARENA_DEBUG
arena: DEBUG += -DARENA_DEBUG
array_mem: DEBUG += -DVALGRIND
unittest_mem: DEBUG += -DVALGRIND
string_mem: DEBUG += -DVALGRIND

# executables definitions
arena: $(ARENA_DEPS)
	$(CC) $(LD_FLAGS) $(ARENA_DEPS) -o $(BUILD_DIR)/$@

arena_mem: $(ARENA_DEPS)
	$(CC) $(LD_FLAGS) $(ARENA_DEPS) -o $(BUILD_DIR)/$@
	valgrind --leak-check=full $(BUILD_DIR)/$@

array: $(ARRAY_DEPS)
	$(CC) $(LD_FLAGS) $(ARRAY_DEPS) -o $(BUILD_DIR)/$@

array_mem: $(ARRAY_DEPS)
	$(CC) $(LD_FLAGS) $(ARRAY_DEPS) -o $(BUILD_DIR)/$@
	valgrind --leak-check=full $(BUILD_DIR)/$@

unittest: $(UNITTEST_DEPS)
	$(CC) $(LD_FLAGS) $(UNITTEST_DEPS) -o $(BUILD_DIR)/$@

unittest_mem: $(UNITTEST_DEPS)
	$(CC) $(LD_FLAGS) $(UNITTEST_DEPS) -o $(BUILD_DIR)/$@
	valgrind --leak-check=full $(BUILD_DIR)/$@

string: $(STRING_DEPS)
	$(CC) $(LD_FLAGS) $(STRING_DEPS) -o $(BUILD_DIR)/$@

string_mem: $(STRING_DEPS)
	$(CC) $(LD_FLAGS) $(STRING_DEPS) -o $(BUILD_DIR)/$@
	valgrind --leak-check=full $(BUILD_DIR)/$@

# Build step for general asm sources
$(BUILD_DIR)/%.o: %.s
	$(ASM) $(ASM_FLAGS) $< -o $@

# Build step for general C sources
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CC_FLAGS) $(DEBUG) -c $< -o $@

.PHONY: clean
clean:
	-rm ./build/*

-include $(DEPS)
