#include <iostream>
#include "assembler.hpp"

int main(int argc, char *argv[]) {
    std::cout << "Started." << "\n";
    return Assembler::singleton().pass(argc, argv);
}