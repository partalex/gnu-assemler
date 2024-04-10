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

$(BIN_PATH):
	mkdir -p $(BIN_PATH)

clean:
	rm -rf $(BIN_PATH)

$(SRC_PATH)/lexer.cpp: $(LEXER)
	flex -o $@ $(LEXER)
# -d for debug

$(SRC_PATH)/parser.cpp: $(PARSER) $(LEXER)
	bison -o $@ $(PARSER)
# -t for debug