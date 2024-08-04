#include "../include/emulator.h"
#include "../../common/include/program.h"

#include <iostream>

std::unique_ptr<Emulator> Emulator::_instance = nullptr;

Emulator &Emulator::singleton() {
    if (!_instance) {
        _instance = std::make_unique<Emulator>();
        _instance->_program = std::make_unique<Program>();
    }
    return *_instance;
}

void Emulator::parseArgs(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "No input file" << '\n';
        exit(EXIT_FAILURE);
    }
    _program = std::make_unique<Program>();
    _program->load(argv[1]);
}

void Emulator::execute() {
    _program->initNew();
    while (true) {
        _program->executeCurrent();
        if (_program->_isEnd)
            break;
        _program->readNext();
    }
}


