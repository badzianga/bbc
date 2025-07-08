CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -ggdb

INC_DIR := include
SRC_DIR := src
OBJ_DIR := obj

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET := bbc

all: $(TARGET)

$(TARGET): $(OBJ_DIR)/bbc.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/bbc.o: bbc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) test test.o test.asm

.PHONY: all clean



test: test.o
	gcc -no-pie test.o -o test

test.o: test.asm
	fasm test.asm

test.asm: $(TARGET) examples/compilable.b
	./$(TARGET) examples/compilable.b
