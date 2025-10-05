# Simple Makefile for ls clone project
CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRC = $(SRC_DIR)/ls-v1.0.0.c
OBJ = $(OBJ_DIR)/ls-v1.0.0.o
BIN = $(BIN_DIR)/ls

.PHONY: all clean run

all: $(BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ): | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

run: all
	./$(BIN)
