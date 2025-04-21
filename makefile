.DELETE_ON_ERROR:
CC = clang
CC_FLAGS = -Wall -MMD -MP -DDEBUG
LD_FLAGS = -lm
DEBUG = -ggdb3
ASM = nasm
ASM_FLAGS = -felf64 -g
LD = ld

BUILD_DIR := build

SRCS := $(shell find . -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

SRC_DIRS := ./tests
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CC_FLAGS += $(INC_FLAGS)

# build allocator for valgrind testing
unittest_mem: DEBUG += -DVALGRIND

unittest: $(OBJS)
	$(CC) $(LD_FLAGS) $(OBJS) -o $(BUILD_DIR)/$@

unittest_mem: $(OBJS)
	$(CC) $(LD_FLAGS) $(OBJS) -o $(BUILD_DIR)/$@
	-valgrind --leak-check=full $(BUILD_DIR)/$@

# Build step for general asm sources
$(BUILD_DIR)/%.o: %.s
	$(ASM) $(ASM_FLAGS) $< -o $@

# Build step for general C sources
$(BUILD_DIR)/%.c.o: %.c
	$(CC) $(CC_FLAGS) $(DEBUG) -c $< -o $@

.PHONY: clean
clean:
	-rm ./build/*
	-rm ./build/tests/*

.PHONY: format
format:
	clang-format -i ./*.c ./*.h ./tests/*.h ./tests/*.c

-include $(DEPS)
