#include "../include/emulator.h"
#include "../../common/include/program.h"

#include <iostream>

std::unique_ptr<Emulator> Emulator::_instance = nullptr;
std::unique_ptr<Program> Emulator::program = nullptr;

Emulator &Emulator::singleton() {
    if (!_instance)
        _instance = std::make_unique<Emulator>();
    return *_instance;
}

void Emulator::parseArgs(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "No input file" << '\n';
        exit(EXIT_FAILURE);
    }
    program = std::make_unique<Program>();
    program->load(argv[1]);
    auto stackSegment = Segment{STACK_START, STACK_SIZE};
    program->memory.insertSegment(stackSegment);
    program->memory.mergeAdjacent();
}

void Emulator::execute() {

//    program->initOld();
    program->initNew();

    while (!program->isEnd) {
        program->executeCurrent();
        program->readNext();
    }

}


