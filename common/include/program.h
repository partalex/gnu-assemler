#pragma once

#include "program_info.h"
#include "instruction.h"

#include <unordered_map>
#include <functional>
#include <chrono>
#include <fstream>

// 1 MB of memory
// code:  [0, 1KB)
// data:  [1KB, 5KB)
// stack: [5KB, 1MB)

#define KB 1024
#define MB (1024 * KB)

#define INSTR_SIZE 4
#define STACK_INCREMENT 4
#define MEMORY_SIZE (1 * MB)

#define TEXT_OFFSET 0
#define TEXT_MAX_SIZE (1 * KB)
#define DATA_MAX_SIZE (4 * KB)
#define STACK_SIZE (6 * KB)

#define START_POINT 0x40000000
#define STACK_START MEMORY_SIZE
#define STACK_END ( STACK_START - STACK_SIZE )

#define DATA_OFFSET TEXT_MAX_SIZE

#define KEYBOARD_POS 0x1000
#define KEYBOARD_STATUS_POS 0x1010
#define KEYBOARD_STATUS_MASK 1L << 9
#define OUTPUT_POS 0x2000

class Program {
public:
    static std::unique_ptr<std::ofstream> LOG;
    std::vector<int32_t> registers = std::vector<int32_t>(19, 0);
    ProgramInfo info{0, 0};
    std::vector<uint8_t> memory;
    uint32_t LR = 0;
    Mnemonic currInstr{0};
    pthread_t keyboardThread;
    std::chrono::time_point<std::chrono::system_clock> executionStart;
    std::chrono::time_point<std::chrono::system_clock> lastTimerExecution;
    std::unordered_map<int, std::function<void()> > instructionExecutors;
    std::unordered_map<int, std::function<bool()> > conditionTesters;
    PSW psw;
    bool isEnd = false;

    Program();

    int32_t &STATUS();

    int32_t &HANDLER();

    int32_t &CAUSE();

    int32_t &PC();

    int32_t &SP();

    void load(const std::string &);

    uint32_t pop();

    void push(uint32_t);

    void getInstr();

    uint32_t getMemoryOffset(uint32_t) const;

    void initOld();

    void initNew();

    void readNext();

    void executeCurrent();

    void logState();

    void handleInterrupts();

    void timerInterrupt();

    static void *KeyboardThread(void *);

    static volatile char keyboardBuf;

    static volatile bool keyBarrier;

    static uint32_t signExt(uint32_t, size_t);

    void setMemory(uint32_t, uint32_t);

    uint32_t getMemory(uint32_t);

    void keyInterr();

    int32_t sum(int32_t, int32_t);

    int32_t div(int32_t, int32_t);

    int32_t mul(int32_t, int32_t);

    int32_t sub(int32_t, int32_t);

    int32_t and_(int32_t, int32_t);

    int32_t or_(int32_t, int32_t);

    int32_t xor_(int32_t, int32_t);

    int32_t not_(int32_t);

    int32_t shl(int32_t, int32_t);

    int32_t shr(int32_t, int32_t);


};