SRC_LN = src
SRC_COMMON = ../common/src
BIN_PATH = bin

SRCS = $(wildcard $(SRC_LN)/*.cpp) $(SRC_COMMON)/*

CC = g++

run: debug
	$(BIN_PATH)/main $(ARGS)

all: $(BIN_PATH)
	$(CC) $(SRCS) -o $(BIN_PATH)/main

debug: $(BIN_PATH)
	$(CC) $(SRCS) -g -o $(BIN_PATH)/main

clean:
	rm -rf $(BIN_PATH)

$(BIN_PATH):
	mkdir -p $(BIN_PATH)