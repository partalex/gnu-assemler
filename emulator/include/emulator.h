#pragma once

#include <fstream>
#include <memory>

class Program;

class Emulator {
    static std::unique_ptr<Emulator> _instance;
    static std::unique_ptr<Program> program;
    std::string inputFile;

public:
    Emulator() = default;

    ~Emulator() = default;

    void operator=(Emulator const &) = delete;

    static Emulator &singleton();

    void parseArgs(int, char **);

    void execute();
};