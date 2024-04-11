INCLUDE_PATH = include
SRC_PATH = src
BIN_PATH = bin

HDRS = $(wildcard $(INCLUDE_PATH)/*.hpp)
SRCS = $(wildcard $(SRC_PATH)/*.cpp)
LEXER = $(SRC_PATH)/lexer.l
PARSER = $(SRC_PATH)/parser.y
CC = g++

all: $(BIN_PATH) $(SRCS) $(HDRS)
	$(CC) -I$(INCLUDE_PATH) $(SRCS) -o bin/main

lexer: $(PARSER)
	bison -o $(SRC_PATH)/parser.cpp $(PARSER)
# -d for debug

parser: $(PARSER)
	bison -o $(SRC_PATH)/parser.cpp $(PARSER)
# -t for debug

clean:
	rm -rf $(BIN_PATH)
	rm -f $(SRC_PATH)/lexer.cpp
	rm -f $(SRC_PATH)/parser.cpp
	rm -f $(INCLUDE_PATH)/parser.hpp
	rm -f $(INCLUDE_PATH)/lexer.hpp

$(BIN_PATH):
	mkdir -p $(BIN_PATH)



