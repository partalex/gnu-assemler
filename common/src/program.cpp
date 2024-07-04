#include "../../common/include/program.h"

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

    // read program link
    file.read(reinterpret_cast<char *>(&info), sizeof(info));
    // read START_POINT
    if (info.startPoint != START_POINT)
        throw std::runtime_error("Invalid start point!");

    // check if text section is too big
    if (info.textSize > TEXT_MAX_SIZE)
        throw std::runtime_error("Text section too big!");

    // check if data section is too big
    if (info.dataSize > DATA_MAX_SIZE)
        throw std::runtime_error("Data section too big!");

    memory.resize(MEMORY_SIZE);

    // read text section
    file.read(reinterpret_cast<char *>(memory.data() + TEXT_OFFSET), info.textSize);

    // read rest of the file to data section
    file.read(reinterpret_cast<char *>(memory.data() + DATA_OFFSET), info.dataSize);
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
    memcpy(memory.data() + SP(), &val, STACK_INCREMENT);
}

uint32_t Program::pop() {
    if (SP() >= STACK_START)
        throw std::runtime_error("Stack underflow!");
    uint32_t ret;
    std::memcpy(&ret, memory.data() + SP(), STACK_INCREMENT);
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
    memcpy(&PC(), memory.data(), INSTR_SIZE);
//    PC() += START_POINT;
}

void Program::getInstr() {
    memcpy(&currInstr, memory.data() + PC() - START_POINT + TEXT_OFFSET, INSTR_SIZE);
}

void Program::initNew() {
    getInstr();
}

void Program::readNext() {
    PC() += INSTR_SIZE;
    if (PC() >= START_POINT + info.textSize)
        throw std::runtime_error("Out of scope! Use halt at the end of program.");
    getInstr();
}

void Program::setMemory(uint32_t val, uint32_t addr) {
    auto offset = getMemoryOffset(addr);
    *LOG << "Set memory " << std::hex << addr << " - " << val << "\n";
    memcpy(memory.data() + offset, &val, sizeof(val));
}

uint32_t Program::getMemory(uint32_t addr) {
    auto offset = getMemoryOffset(addr);
    uint32_t res;
    memcpy(&res, memory.data() + offset, sizeof(res));
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
    if (memory[OUTPUT_POS] != 0) {
        auto temp = memory[OUTPUT_POS];
        memory[OUTPUT_POS] = 0;
        std::cout << temp;
        std::cout.flush();
        *LOG << "Print char: " << temp << '\n';
    }
}

int32_t &Program::STATUS() {
    return registers[CSR::STATUS];
}

int32_t &Program::HANDLER() {
    return registers[CSR::HANDLER];
}

int32_t &Program::CAUSE() {
    return registers[CSR::CAUSE];
}

int32_t &Program::PC() {
    return registers[15];
}

int32_t &Program::SP() {
    return registers[14];
}

void Program::keyInterr() {
    if (psw.I == 1) {
        *LOG << "Masked interrupts (keyboard)" << '\n';
        return;
    }
    *LOG << "Keyboard interrupt!" << '\n';
    auto temp = keyboardBuf;
    keyBarrier = false;
    memcpy(memory.data() + KEYBOARD_POS, &temp, 4);
    auto mask = KEYBOARD_STATUS_MASK;
    memcpy(memory.data() + KEYBOARD_STATUS_POS, &mask, 4);
    push(LR);
    LR = PC();
    push(psw.val);
    psw.I = 1;
    memcpy(&PC(), memory.data() + 12, 4);
    PC() += START_POINT;
}

void Program::timerInterrupt() {
    if (psw.I == 1) {
        *LOG << "Masked interrupts (timer)" << '\n';
        return;
    }
    if (psw.Tr == 0) {
        *LOG << "Masked timer interrupt" << '\n';
        return;
    }
    *LOG << "Timer interrupt!" << '\n';
    push(LR);
    LR = PC();
    push(psw.val);
    psw.I = 1;
    memcpy(&PC(), memory.data() + 4, 4);
    PC() += START_POINT;
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

uint32_t Program::getMemoryOffset(uint32_t addr) const {
//    if (addr >= info.dataAddr + info.dataSize)
    if (addr >= info.dataAddr + DATA_MAX_SIZE)
        throw std::runtime_error("Index for memory out of bounds!");
    return addr - info.dataAddr;
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