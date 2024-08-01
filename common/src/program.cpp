#include "../include/program.h"

#include <iostream>
#include <cstring>
#include <cstdint>
#include <iomanip>

volatile char Program::keyboardBuf;
volatile bool Program::keyBarrier;
std::unique_ptr<std::ofstream> Program::LOG = nullptr;

Program::Program() {
    LOG = std::make_unique<std::ofstream>("log.txt");
    PC() = START_POINT;
    SP() = STACK_START;
    if (!LOG->is_open())
        throw std::runtime_error("Could not open log file!");
}

void Program::load(const std::string &inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open())
        throw std::runtime_error("Could not open file " + inputFile);

    uint32_t numSections;
    file.read(reinterpret_cast<char *>(&numSections), sizeof(numSections));
    for (uint32_t i = 0; i < numSections; ++i) {
        uint32_t sectionAddr;
        file.read(reinterpret_cast<char *>(&sectionAddr), sizeof(sectionAddr));
        uint32_t sectionSize;
        file.read(reinterpret_cast<char *>(&sectionSize), sizeof(sectionSize));
        auto segment = Segment{sectionAddr, sectionSize};
        file.read(reinterpret_cast<char *>(segment.data.data()), sectionSize);
        memory.insertSegment(segment);
    }
    file.close();
}

uint32_t Program::signExt(uint32_t val, size_t size) {
    val <<= 32 - size;
    auto temp = val;
    temp >>= 32 - size;
    val = temp;
    return val;
}

void Program::push(uint32_t val) {
    if (SP() <= STACK_END)
        throw std::runtime_error("Stack overflow!");
    *LOG << "Stack push " << std::hex << val << '\n';
    SP() -= STACK_INCREMENT;
    memory.writeWord(SP(), val);
}

uint32_t Program::pop() {
    if (SP() >= STACK_START)
        throw std::runtime_error("Stack underflow!");
    auto ret = memory.readWord(SP());
    SP() += STACK_INCREMENT;
    *LOG << "Stack pop " << std::hex << ret << '\n';
    return ret;
}

void Program::initOld() {
    PC() = START_POINT;
    SP() = STACK_START;
    psw.val = 0;
    psw.Tr = 1;
    executionStart = std::chrono::system_clock::now();
    lastTimerExecution = executionStart;
    int iRet1 = pthread_create(&keyboardThread, nullptr, KeyboardThread, nullptr);
    if (iRet1)
        throw std::runtime_error(&"Error - pthread_create() return code: "[iRet1]);
    //call first routine
    push(LR);
    LR = PC();
    push(psw.val);
    psw.I = 1;
//    memcpy(&PC(), memory.data(), INSTR_SIZE);
    PC() = memory.readWord(0);
//    PC() += START_POINT;
}

void Program::getInstr() {
    currInstr.value = memory.readWord(PC());
}

void Program::initNew() {
    getInstr();
}

void Program::readNext() {
    PC() += INSTR_SIZE;
    getInstr();
}

void Program::setMemory(uint32_t val, uint32_t addr) {
    *LOG << "Set memory " << std::hex << addr << " - " << val << "\n";
    memory.writeWord(addr, val);
}

uint32_t Program::getMemory(uint32_t addr) {
    uint32_t res = memory.readWord(addr);
    *LOG << "Fetched memory from " << std::hex << addr << " - " << res << '\n';
    return res;
}

void Program::logState() {
    for (int i = 0; i < 16; i += 4) {
        for (int j = 0; j < 4; ++j) {
            *LOG << "r" << std::right << std::setfill(' ')
                 << std::setw(2) << std::left
                 << std::dec << i + j << "=0x"
                 << std::setfill('0') << std::setw(8)
                 << std::hex << registers[i + j] << " ";
        }
        *LOG << '\n';
    }
    *LOG << "PC =0x" << std::setfill('0') << std::setw(8) << std::hex << PC() << " "
         << "LR =0x" << std::setfill('0') << std::setw(8) << std::hex << LR << " "
         << "SP =0x" << std::setfill('0') << std::setw(8) << std::hex << SP() << " "
         << "psw=0x" << std::setfill('0') << std::setw(8) << std::hex << psw.val << '\n';
    *LOG << "STATUS =0x" << std::setfill('0') << std::setw(8) << std::hex << registers[16] << " "
         << "HANDLER =0x" << std::setfill('0') << std::setw(8) << std::hex << registers[17] << " "
         << "CAUSE =0x" << std::setfill('0') << std::setw(8) << std::hex << registers[18] << '\n';
//    uint32_t Tr: 1, Tl: 1, I: 1, : 24, Z: 1, O: 1, C: 1, N: 1;

    *LOG << "TR=" << psw.Tr << " TL=" << psw.Tl << " I=" << psw.I <<
         " Z=" << psw.Z << " O=" << psw.O << " C=" << psw.C << " N=" << psw.N <<
         '\n';
}

void Program::executeCurrent() {
    logState();
    Instruction::execute(currInstr, *this);
    logState();
//    handleInterrupts();
    *LOG << '\n';
}

void Program::handleInterrupts() {
    auto timeNow = std::chrono::system_clock::now();
    auto elapsed = timeNow - lastTimerExecution;
    //TODO: hardcoded
    if (elapsed.count() >= 1.0) {
        lastTimerExecution = timeNow;
        timerInterrupt();
    }
    if (keyBarrier)
        keyInterr();
    // TODO
    auto state = memory.readWord(OUTPUT_POS);
    if (state != 0) {
        memory.writeWord(OUTPUT_POS, 0);
        std::cout << state;
        std::cout.flush();
        *LOG << "Print char: " << state << '\n';
    }
}

int32_t &Program::STATUS() {
    return registers[REG_CSR::CSR_STATUS];
}

int32_t &Program::HANDLER() {
    return registers[REG_CSR::CSR_HANDLER];
}

int32_t &Program::CAUSE() {
    return registers[REG_CSR::CSR_CAUSE];
}

int32_t &Program::PC() {
    return registers[15];
}

int32_t &Program::SP() {
    return registers[14];
}

void Program::keyInterr() {
    if (psw.I) {
        *LOG << "Masked interrupts (keyboard)" << '\n';
        return;
    }
    *LOG << "Keyboard interrupt!" << '\n';
    auto temp = keyboardBuf;
    keyBarrier = false;
    memory.writeWord(KEYBOARD_POS, temp);
    auto mask = KEYBOARD_STATUS_MASK;
    memory.writeWord(KEYBOARD_STATUS_POS, mask);
    push(LR);
    LR = PC();
    push(psw.val);
    psw.I = 1;
    // TODO
//    memcpy(&PC(), memory.data() + 12, 4);
//    PC() += START_POINT;
}

void Program::timerInterrupt() {
    if (psw.I) {
        *LOG << "Masked interrupts (timer)" << '\n';
        return;
    }
    if (!psw.Tr) {
        *LOG << "Masked timer interrupt" << '\n';
        return;
    }
    *LOG << "Timer interrupt!" << '\n';
    push(LR);
    LR = PC();
    push(psw.val);
    psw.I = 1;
    // TODO
//    memcpy(&PC(), memory.data() + 4, 4);
//    PC() += START_POINT;
}

void *Program::KeyboardThread(void *) {
    while (true) {
        while (keyBarrier);
        *LOG << "Passed barrier" << '\n';
        char temp;
        std::cin >> std::noskipws >> temp;
        *LOG << "Read char " << temp << '\n';
        keyboardBuf = temp;
        keyBarrier = true;
    }
}

int32_t Program::sum(const int32_t val1, const int32_t val2) {
    int64_t temp = static_cast<int64_t>(val1) + static_cast<int64_t>(val2);
    psw.Z = (temp == 0);
    psw.N = (temp < 0);
    psw.C = (val1 > 0 && val2 > 0 && temp < 0) || (val1 < 0 && val2 < 0 && temp >= 0);
    psw.O = ((val1 < 0) == (val2 < 0)) && ((temp < 0) != (val1 < 0));
    return static_cast<int32_t>(temp);
}

int32_t Program::div(int32_t val1, int32_t val2) {
    if (val2 == 0) throw std::runtime_error("Division by zero!");
    int32_t result = val1 / val2;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = false;
    psw.O = (val1 == INT32_MIN && val2 == -1);
    return result;
}

int32_t Program::mul(int32_t val1, int32_t val2) {
    int64_t temp = static_cast<int64_t>(val1) * static_cast<int64_t>(val2);
    psw.Z = (temp == 0);
    psw.N = (temp < 0);
    psw.C = (temp != static_cast<int32_t>(temp));
    psw.O = psw.C;
    return static_cast<int32_t>(temp);
}

int32_t Program::sub(int32_t val1, int32_t val2) {
    int64_t temp = static_cast<int64_t>(val1) - static_cast<int64_t>(val2);
    psw.Z = (temp == 0);
    psw.N = (temp < 0);
    psw.C = (val1 < val2);
    psw.O = ((val1 < 0) != (val2 < 0)) && ((temp < 0) != (val1 < 0));
    return static_cast<int32_t>(temp);
}

int32_t Program::and_(int32_t val1, int32_t val2) {
    int32_t result = val1 & val2;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = psw.O = false;
    return result;
}

int32_t Program::or_(int32_t val1, int32_t val2) {
    int32_t result = val1 | val2;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = psw.O = false;
    return result;
}

int32_t Program::xor_(int32_t val1, int32_t val2) {
    int32_t result = val1 ^ val2;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = psw.O = false;
    return result;
}

int32_t Program::not_(const int32_t val) {
    int32_t result = ~val;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = psw.O = false;
    return result;
}

int32_t Program::shl(const int32_t val, const int32_t n) {
    int32_t result = val << n;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = (val & (1 << (31 - n))) != 0;
    psw.O = false;
    return result;
}

int32_t Program::shr(const int32_t val, const int32_t n) {
    int32_t result = val >> n;
    psw.Z = (result == 0);
    psw.N = (result < 0);
    psw.C = (val & (1 << (n - 1))) != 0;
    psw.O = false;
    return result;
}