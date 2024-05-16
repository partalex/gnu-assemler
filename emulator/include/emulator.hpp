#include <fstream>

#include "program.hpp"

class Emulator {
public:
    Emulator();

    void Emulate(std::ifstream &inputFiles);

    static std::ofstream logFile;

private:
    void Execute();

    Program program;

};