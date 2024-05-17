BIN_PATH = bin

SRCS = src/*
CC = g++

run: debug
	$(BIN_PATH)/main

all: $(BIN_PATH)
	$(CC) $(SRCS) -o $(BIN_PATH)/main

debug: $(BIN_PATH)
	$(CC) $(SRCS) main.cpp -g -o $(BIN_PATH)/main

clean:
	rm -rf $(BIN_PATH)

$(BIN_PATH):
	mkdir -p $(BIN_PATH)