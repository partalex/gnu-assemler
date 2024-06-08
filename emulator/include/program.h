#pragma once

#include "../../common/include/instruction.h"
#include "../../common/include/section.h"

#include <istream>
#include <chrono>
#include <functional>
#include <unordered_map>

class Instruction;

class Program {
public:
    Program();

    void loadSection(std::istream &);

    void loadDefaultIVT(std::ifstream &);

    Instruction currInstr;

    static const size_t MEMORY_SIZE = 1024 * 1024;
    static const size_t STACK_START = 128 * 1024;
    static const size_t STACK_SIZE = 8 * 1024;
    static const size_t IVT_SIZE = 1024;
    static const size_t IVT_START = 256 * 1024;
    static const size_t KEYBOARD_POS = 0x1000;
    static const size_t KEYBOARD_STATUS_POS = 0x1010;
    static const uint32_t KEYBOARD_STATUS_MASK = 1L << 9;
    static const size_t OUTPUT_POS = 0x2000;
    static const size_t IRET_CODE = 0x230ee;
    static const size_t CODE_START = 16 * 4;

    uint8_t memory[MEMORY_SIZE];
    std::unordered_map<int, std::function<void()> > instructionExecutors;
    std::unordered_map<int, std::function<bool()> > conditionTesters;

    union {
        uint32_t val;
        struct {
            uint32_t I: 1, : 1, T: 1, : 25, Z: 1, O: 1, C: 1, N: 1;
        };
    } PSW;

    uint32_t registers[16];
    uint32_t startPoint;

    uint32_t stackPop();

    uint32_t getRegister(uint32_t);

    uint32_t PC, LR, SP;

    Section programSection;

    void init();

    void stackPush(uint32_t);

    void readNext();

    void executeCurrent();

    bool isEnd();

    void handleInterrupts();

    void timerInterrupt();

    static void *KeyboardThread(void *);

    pthread_t keyboardThread;

    std::chrono::time_point<std::chrono::system_clock> executionStart;
    std::chrono::time_point<std::chrono::system_clock> lastTimerExecution;

    static volatile char keyboardBuf;
    static volatile bool keyboardInterrupt;

    void setRegister(uint32_t, uint32_t);

    void setMemory(uint32_t, uint32_t);

    uint32_t getMemory(uint32_t );

    uint32_t signExt(uint32_t, size_t);

    void KeyboardInterrupt();
};