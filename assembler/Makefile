INCLUDE_AS = include
SRC_AS = src
SRC_COMMON = ../common/src
BIN_PATH = bin

SRCS = $(wildcard $(SRC_AS)/*.cpp) $(SRC_COMMON)/*

LEXER = src/lexer.l
PARSER = src/parser.y
ARGS = -o $(SRC_AS)/izlaz.o ../test/handler.s
CC = g++

debug: $(BIN_PATH) parser
	$(CC) -I$(INCLUDE_AS) $(SRCS) -g -o $(BIN_PATH)/main

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