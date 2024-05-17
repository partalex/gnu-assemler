#include <iostream>
#include "emulator.h"

Emulator::Emulator() = default;

void Emulator::Emulate(std::ifstream &inputFiles) {
    try {
        logFile << "Loading section" << std::endl;
        program.loadSection(inputFiles);
        std::ifstream defaultIVT("object_files/defaultIVT.o");
        logFile << "Load default ivt" << std::endl;
        program.loadDefaultIVT(defaultIVT);
        logFile << "Started executing" << std::endl;
        Execute();
    }

    catch (std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        logFile << "ERROR: " << exc.what() << std::endl;
    }
}

void Emulator::Execute() {
    program.init();
    program.readNext();
    while (!program.isEnd()) {
        program.executeCurrent();
        program.readNext();
    }
}
