SRC = src
SRC_COMMON = ../common/src
BIN_PATH = bin

SRCS = $(SRC)/* $(SRC_COMMON)/* ../assembler/src/assembler.cpp ../assembler/src/parser.cpp ../assembler/src/lexer.cpp

CC = g++

debug: $(BIN_PATH)
	$(CC) -I../assembler/include $(SRCS) -g -o $(BIN_PATH)/main

clean:
	rm -rf $(BIN_PATH)

$(BIN_PATH):
	mkdir -p $(BIN_PATH)