#pragma once

#include "instruction.h"
#include "memory.h"

#include <unordered_map>
#include <functional>
#include <chrono>
#include <fstream>
#include <set>

class Program {
public:
    static std::unique_ptr<std::ofstream> LOG;
    std::vector<int32_t> gpr_registers = std::vector<int32_t>(15, 0);
    std::vector<int32_t> csr_registers = std::vector<int32_t>(3, 0);
    Mnemonic currInstr{0};
    pthread_t keyboardThread;
    std::chrono::time_point<std::chrono::system_clock> executionStart;
    std::chrono::time_point<std::chrono::system_clock> lastTimerExecution;
    std::unordered_map<int, std::function<void()> > instructionExecutors;
    std::unordered_map<int, std::function<bool()> > conditionTesters;

    Memory memory;
    PSW psw;
    bool _isEnd = false;
    bool _incrementPC = true;
    uint32_t _instrCounter;

    explicit Program();

    int32_t &STATUS();

    int32_t &HANDLER();

    int32_t &CAUSE();

    int32_t &PC();

    int32_t &SP();

    void load(const std::string &);

    int32_t pop();

    int32_t castToSign(int32_t, uint8_t);

    int32_t displacement();

    void push(int32_t);

    void loadInstr();

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

    void setMemory(uint32_t, int32_t);

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