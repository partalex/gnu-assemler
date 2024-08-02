#pragma once

#include "instruction.h"
#include "memory.h"

#include <unordered_map>
#include <functional>
#include <chrono>
#include <fstream>
#include <set>

// 1 MB of memory
// code:  [0, 1KB)
// data:  [1KB, 5KB)
// stack: [5KB, 1MB)

#define KB 1024
#define MB (1024 * KB)

#define INSTR_SIZE 4
#define STACK_INCREMENT 4
#define START_POINT 0x40000000

#define STACK_SIZE (6 * KB)
#define STACK_START STACK_SIZE
#define STACK_END ( STACK_START - STACK_SIZE ) // 0x00000000

#define KEYBOARD_POS 0x1000
#define KEYBOARD_STATUS_POS 0x1010
#define KEYBOARD_STATUS_MASK 1L << 9
#define OUTPUT_POS 0x2000

class Program {
public:
    static std::unique_ptr<std::ofstream> LOG;
    std::vector<int32_t> gpr_registers = std::vector<int32_t>(15, 0);
    std::vector<int32_t> csr_registers = std::vector<int32_t>(3, 0);
    uint32_t LR = 0;
    Mnemonic currInstr{0};
    pthread_t keyboardThread;
    std::chrono::time_point<std::chrono::system_clock> executionStart;
    std::chrono::time_point<std::chrono::system_clock> lastTimerExecution;
    std::unordered_map<int, std::function<void()> > instructionExecutors;
    std::unordered_map<int, std::function<bool()> > conditionTesters;

    Memory memory;

    PSW psw;

    bool isEnd = false;
    bool incrementPC = true;
    uint32_t instrCounter = 0;

    Program();

    int32_t &STATUS();

    int32_t &HANDLER();

    int32_t &CAUSE();

    int32_t &PC();

    int32_t &SP();

    void load(const std::string &);

    int32_t pop();

    void push(int32_t);

    void getInstr();

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

    int32_t getMemory(uint32_t);

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