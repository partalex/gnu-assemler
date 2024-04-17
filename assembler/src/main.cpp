#include "assembler.hpp"

int main(int argc, char *argv[]) {
    return Assembler::singleton().pass(argc, argv);
}