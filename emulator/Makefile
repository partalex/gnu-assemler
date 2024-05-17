INCLUDE = include
SRC_EM = src
SRC_COMMON = ../common/src
BIN_PATH = bin

SRCS = $(wildcard $(SRC_LN)/*.cpp) $(SRC_COMMON)/*

ARGS = -o $(SRC_LN)/izlaz.o ../test/handler.s
CC = g++

run: debug
	$(BIN_PATH)/main $(ARGS)

all: $(BIN_PATH)
	$(CC) -I$(INCLUDE) $(SRCS) -o $(BIN_PATH)/main

debug: $(BIN_PATH)
	$(CC) -I$(INCLUDE) -I$(INCLUDE_COMMON) $(SRCS) -g -o $(BIN_PATH)/main

clean:
	rm -rf $(BIN_PATH)

$(BIN_PATH):
	mkdir -p $(BIN_PATH)