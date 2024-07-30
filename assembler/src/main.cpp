#include "../include/assembler.h"

int main(int argc, char *argv[]) {
    return Assembler::singleton().pass(argc, argv);
}
