.PHONY: all run debug lexer parser clean

INCLUDE_AS = include
INCLUDE_COMMON = ../common/include

SRC_AS = src
SRC_COMMON = ../common/src

BIN_PATH = bin

SRCS = $(wildcard $(SRC_AS)/*.cpp) $(SRC_COMMON)/*

LEXER = src/lexer.l
PARSER = src/parser.y
ARGS = -o $(SRC_AS)/izlaz.o ../test/handler.s
CC = g++

run: debug
	$(BIN_PATH)/main $(ARGS)

all: $(BIN_PATH) parser
	$(CC) -I$(INCLUDE_AS) -I$(INCLUDE_COMMON) $(SRCS) -o $(BIN_PATH)/main

debug: $(BIN_PATH) parser
	$(CC) -I$(INCLUDE_AS) -I$(INCLUDE_COMMON) $(SRCS) -g -o $(BIN_PATH)/main

lexer: $(LEXER)
	flex -o src/lexer.cpp $(LEXER)
# -d for debug

parser: lexer $(PARSER)
	bison -o src/parser.cpp $(PARSER)
# -t for debug

clean:
	rm -rf $(BIN_PATH)
	rm -f $(SRC_AS)/lexer.cpp
	rm -f $(SRC_AS)/parser.cpp
	rm -f $(INCLUDE_AS)/parser.h
	rm -f $(INCLUDE_AS)/lexer.h

$(BIN_PATH):
	mkdir -p $(BIN_PATH)