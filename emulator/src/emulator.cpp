#include "../include/emulator.h"
#include "../../common/include/program.h"

#include <iostream>

std::unique_ptr<Emulator> Emulator::instance = nullptr;

Emulator &Emulator::singleton() {
    if (!instance) {
        instance = std::make_unique<Emulator>();
        instance->program = std::make_unique<Program>();
    }
    return *instance;
}

void Emulator::parseArgs(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "No input file" << '\n';
        exit(EXIT_FAILURE);
    }
    program = std::make_unique<Program>();
    program->load(argv[1]);
}

void Emulator::execute() {
    program->initNew();
    while (true) {
        program->executeCurrent();
        if (program->isEnd)
            break;
        program->readNext();
        program->setReg0();
    }
}


