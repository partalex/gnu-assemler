#pragma once

#include <fstream>
#include <memory>

static constexpr auto KB = 1024;
static constexpr auto MB = 1024 * KB;
static constexpr auto GB = 1024 * MB;
static constexpr auto DEFAULT_PC = 0x40000000;
static constexpr auto DEFAULT_SP = 4 * KB;
static constexpr auto MEM_SIZE = GB * 4ULL;
static constexpr auto MIN_ADDRESS = 0;
static constexpr auto MAX_ADDRESS = MIN_ADDRESS + MEM_SIZE;
static constexpr auto SEGMENT_SIZE = KB;
static constexpr auto STACK_INCREMENT = 4;
static constexpr auto INSTR_SIZE = 4;
static constexpr auto KEYBOARD_POS = 0x1000;
static constexpr auto KEYBOARD_STATUS_POS = 0x1010;
static constexpr auto KEYBOARD_STATUS_MASK = 1L << 9;
static constexpr auto OUTPUT_STATUS_POS = 0x2010;

class Program;

class Emulator {
    static std::unique_ptr<Emulator> instance;
    std::unique_ptr<Program> program;
    std::string inputFile;
public:

    void operator=(Emulator const &) = delete;

    static Emulator &singleton();

    void parseArgs(int, char **);

    void execute();


};