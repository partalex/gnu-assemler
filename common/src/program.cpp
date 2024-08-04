#include "../include/program.h"
#include "../../emulator/include/emulator.h"

#include <iostream>
#include <cstring>
#include <cstdint>
#include <iomanip>

volatile char Program::keyboardBuf;
volatile bool Program::keyBarrier;
std::unique_ptr<std::ofstream> Program::LOG = nullptr;

Program::Program() : memory(MIN_ADDRESS, MEM_SIZE, SEGMENT_SIZE) {
    LOG = std::make_unique<std::ofstream>("log.txt");
    PC() = DEFAULT_PC;
    SP() = DEFAULT_SP;
    if (!LOG->is_open())
        throw std::runtime_error("Could not open log file!");
}


void Program::load(const std::string &inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open())
        throw std::runtime_error("Could not open file " + inputFile);

    uint32_t numSections;
    auto tempVector = std::vector<uint8_t>();
    file.read(reinterpret_cast<char *>(&numSections), sizeof(numSections));
    for (uint32_t i = 0; i < numSections; ++i) {
        uint32_t startAddr;
        file.read(reinterpret_cast<char *>(&startAddr), sizeof(startAddr));
        uint32_t segmentSize;
        file.read(reinterpret_cast<char *>(&segmentSize), sizeof(segmentSize));
        tempVector.resize(segmentSize);
        file.read(reinterpret_cast<char *>(tempVector.data()), segmentSize);
        memory.loadMemory(startAddr, tempVector);
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

void Program::push(int32_t val) {
    if (SP() < memory._minAddr)
        throw std::runtime_error("Stack overflow!");
    *LOG << "Stack push " << std::hex << val << '\n';
    SP() -= STACK_INCREMENT;
    memory.writeWord(SP(), val);
}

int32_t Program::pop() {
    if (SP() >= memory._minAddr + memory._size)
        throw std::runtime_error("Stack underflow!");
    auto ret = memory.readWord(SP());
    SP() += STACK_INCREMENT;
    *LOG << "Stack pop " << std::hex << ret << '\n';
    return ret;
}

void Program::initOld() {
    psw.val = 0;
    psw.Tr = 1;
    executionStart = std::chrono::system_clock::now();
    lastTimerExecution = executionStart;
    int iRet1 = pthread_create(&keyboardThread, nullptr, KeyboardThread, nullptr);
    if (iRet1)
        throw std::runtime_error(&"Error - pthread_create() return code: "[iRet1]);
    //call first routine
//    push(LR);
//    LR = PC();
    push(psw.val);
    psw.I = 1;
    PC() = memory.readWord(0);
}

void Program::loadInstr() {
    currInstr.value = memory.readWord(PC());
}

void Program::initNew() {
    loadInstr();
}

void Program::readNext() {
    if (_incrementPC)
        PC() += INSTR_SIZE;
    INSTR_SIZE;
    _incrementPC = true;
    loadInstr();
}

void Program::setMemory(uint32_t addr, int32_t val) {
    *LOG << "Set memory: [0x" << std::hex << addr << "] = " << val << "\n";
    memory.writeWord(addr, val);
}

int32_t Program::getMemory(uint32_t addr) {
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
                 << std::hex << gpr_registers[i + j] << " ";
        }
        *LOG << '\n';
    }
    *LOG << "PC =0x" << std::setfill('0') << std::setw(8) << std::hex << PC() << " "
//         << "LR =0x" << std::setfill('0') << std::setw(8) << std::hex << LR << " "
         << "SP =0x" << std::setfill('0') << std::setw(8) << std::hex << SP() << " "
         << "psw=0x" << std::setfill('0') << std::setw(8) << std::hex << psw.val << '\n';
    *LOG << "STATUS =0x" << std::setfill('0') << std::setw(8) << std::hex << csr_registers[16] << " "
         << "HANDLER =0x" << std::setfill('0') << std::setw(8) << std::hex << csr_registers[17] << " "
         << "CAUSE =0x" << std::setfill('0') << std::setw(8) << std::hex << csr_registers[18] << '\n';
//    uint32_t Tr: 1, Tl: 1, I: 1, : 24, Z: 1, O: 1, C: 1, N: 1;

    *LOG << "TR=" << psw.Tr << " TL=" << psw.Tl << " I=" << psw.I <<
         " Z=" << psw.Z << " O=" << psw.O << " C=" << psw.C << " N=" << psw.N <<
         '\n';
}

void Program::executeCurrent() {
    logState();
    int32_t temp;
    auto code = (INSTRUCTION) currInstr.byte_0;
    switch (code) {
        case HALT:              // halt
            _isEnd = true;
            break;
        case INT:               // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handler;
            push(STATUS());
            push(PC());
            CAUSE() = STATUS::SOFTWARE;
            STATUS() &= ~0x1;
            PC() = HANDLER();
            _incrementPC = false;
            break;
        case CALL:              // push pc; pc<=gpr[A=PC]+gpr[B=0]+D
            push(PC());
            PC() = gpr_registers[currInstr.REG_A] + gpr_registers[currInstr.REG_B] + displacement();
            _incrementPC = false;
            break;
        case CALL_MEM:         // push pc; pc<=memory[gpr[A=PC]+gpr[B=0]+D]
            push(PC());
            PC() = getMemory(
                    gpr_registers[currInstr.REG_A] + gpr_registers[currInstr.REG_B] + displacement());
            _incrementPC = false;
            break;
        case JMP:               // pc<=gpr[A=PC]+D
            PC() = gpr_registers[currInstr.REG_A] + displacement();
            _incrementPC = false;
            break;
        case BEQ :               // if (gpr[B] == gpr[C]) pc<=gpr[A=PC]+D
            if (gpr_registers[currInstr.REG_B] == gpr_registers[currInstr.REG_C])
                PC() = gpr_registers[currInstr.REG_A] + displacement();
            _incrementPC = false;
            break;
        case BNE:               // if (gpr[B] != gpr[C]) pc<=gpr[A=PC]+D
            if (gpr_registers[currInstr.REG_B] != gpr_registers[currInstr.REG_C])
                PC() = gpr_registers[currInstr.REG_A] + displacement();
            _incrementPC = false;
            break;
        case BGT:             // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D
            if (gpr_registers[currInstr.REG_B] > gpr_registers[currInstr.REG_C])
                PC() = gpr_registers[currInstr.REG_A] + displacement();
            _incrementPC = false;
            break;
        case JMP_MEM:           // pc<=memory[gpr[A]+D]
            PC() = getMemory(gpr_registers[currInstr.REG_A] + displacement());
            _incrementPC = false;
            break;
        case BEQ_MEM:           // if (gpr[B] == gpr[C]) pc<=memory[gpr[A=PC]+D]
            if (gpr_registers[currInstr.REG_B] == gpr_registers[currInstr.REG_C])
                PC() = getMemory(gpr_registers[currInstr.REG_A] + displacement());
            _incrementPC = false;
            break;
        case BNE_MEM:          // if (gpr[B] != gpr[C]) pc<=memory[gpr[A=PC]+D]
            if (gpr_registers[currInstr.REG_B] != gpr_registers[currInstr.REG_C])
                PC() = getMemory(gpr_registers[currInstr.REG_A] + displacement());
            _incrementPC = false;
            break;
        case BGT_MEM:          // if (gpr[B] signed> gpr[C]) pc<=memory[gpr[A=PC]+D]
            if (gpr_registers[currInstr.REG_B] > gpr_registers[currInstr.REG_C])
                PC() = getMemory(gpr_registers[currInstr.REG_A] + displacement());
            _incrementPC = false;
            break;
        case XCHG:              // temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp;
            temp = gpr_registers[currInstr.REG_B];
            gpr_registers[currInstr.REG_B] = gpr_registers[currInstr.REG_C];
            gpr_registers[currInstr.REG_C] = temp;
            break;
        case ADD:              // gpr[A]<=gpr[B]+gpr[C]
            gpr_registers[currInstr.REG_A] = sum(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case SUB:               // gpr[A]<=gpr[B]-gpr[C]
            gpr_registers[currInstr.REG_A] = sub(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case MUL:              // gpr[A]<=gpr[B] * gpr[C]
            gpr_registers[currInstr.REG_A] = mul(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case DIV:             // gpr[A]<=gpr[B] / gpr[C]
            gpr_registers[currInstr.REG_A] = div(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case NOT:             // gpr[A]<=~gpr[B]
            gpr_registers[currInstr.REG_A] = not_(gpr_registers[currInstr.REG_B]);
            break;
        case AND:              // gpr[A]<=gpr[B] & gpr[C]
            gpr_registers[currInstr.REG_A] = gpr_registers[currInstr.REG_B] & gpr_registers[currInstr.REG_C];
            break;
        case OR:               // gpr[A]<=gpr[B] | gpr[C]
            gpr_registers[currInstr.REG_A] = or_(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case XOR:               // gpr[A]<=gpr[B] ^ gpr[C]
            gpr_registers[currInstr.REG_A] = xor_(gpr_registers[currInstr.REG_B],
                                                  gpr_registers[currInstr.REG_C]);
            break;
        case SHL:               // gpr[A]<=gpr[B] << gpr[C]
            gpr_registers[currInstr.REG_A] = shl(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case SHR:              // gpr[A]<=gpr[B] >> gpr[C]
            gpr_registers[currInstr.REG_A] = shr(gpr_registers[currInstr.REG_B],
                                                 gpr_registers[currInstr.REG_C]);
            break;
        case ST:                // memory[gpr[A]+gpr[B]+D]<=gpr[C]
            setMemory(gpr_registers[currInstr.REG_A] + gpr_registers[currInstr.REG_B] + displacement(),
                      gpr_registers[currInstr.REG_C]);
            break;
        case ST_IND:            // memory[memory[gpr[A]+gpr[B]+D]]<=gpr[C]
            setMemory(
                    getMemory(gpr_registers[currInstr.REG_A] + gpr_registers[currInstr.REG_B] + displacement()),
                    gpr_registers[currInstr.REG_C]);
            break;
        case ST_POST_INC:     // gpr[A]<=gpr[A]+D; memory[gpr[A]]<=gpr[C] // PUSH
            gpr_registers[currInstr.REG_A] = gpr_registers[currInstr.REG_A] + displacement();
            setMemory(gpr_registers[currInstr.REG_A], gpr_registers[currInstr.REG_C]);
            break;
        case LD_CSR:            // gpr[A]<=csr[B] ## CSRRD
            gpr_registers[currInstr.REG_A] = gpr_registers[currInstr.REG_B];
            break;
        case LD:                // gpr[A]<=gpr[B]+D
            gpr_registers[currInstr.REG_A] = gpr_registers[currInstr.REG_B] + displacement();
            break;
        case LD_IND:           // gpr[A]<=memory[gpr[B]+gpr[C]+D]
            gpr_registers[currInstr.REG_A] =
                    gpr_registers[currInstr.REG_B] + gpr_registers[currInstr.REG_C] + displacement();
            break;
        case LD_POST_INC:       // gpr[A]<=memory[gpr[B]]; gpr[B]<=gpr[B]+D ## POP, RET
            gpr_registers[currInstr.REG_A] = getMemory(gpr_registers[currInstr.REG_B]);
            gpr_registers[currInstr.REG_B] = gpr_registers[currInstr.REG_B] + displacement();
            break;
        case CSR_LD:            // csr[A]<=gpr[B] ## CSRWR
            csr_registers[currInstr.REG_A] = gpr_registers[currInstr.REG_B];
            break;
        case CSR_LD_OR:        // csr[A]<=csr[B]|D
            csr_registers[currInstr.REG_A] = csr_registers[currInstr.REG_B] | displacement();
            break;
        case CSR_LD_IND:       // csr[A]<=memory[gpr[B]+gpr[C]+D]
            csr_registers[currInstr.REG_A] =
                    getMemory(gpr_registers[currInstr.REG_B] + gpr_registers[currInstr.REG_C] + displacement());
            break;
        case CSR_LD_POST_INC:   // csr[A]<=memory[gpr[B]]; gpr[B]<=gpr[B]+D
            csr_registers[currInstr.REG_A] = getMemory(gpr_registers[currInstr.REG_B]);
            gpr_registers[currInstr.REG_B] = gpr_registers[currInstr.REG_B] + displacement();
            break;
        default:
            throw std::runtime_error("Unknown instruction " + std::to_string(currInstr.value));
    }
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
    auto state = memory.readWord(OUTPUT_STATUS_POS);
    if (state != 0) {
        memory.writeWord(OUTPUT_STATUS_POS, 0);
        std::cout << state;
        std::cout.flush();
        *LOG << "Print char: " << state << '\n';
    }
}

int32_t &Program::STATUS() {
    return csr_registers[REG_CSR::CSR_STATUS];
}

int32_t &Program::HANDLER() {
    return csr_registers[REG_CSR::CSR_HANDLER];
}

int32_t &Program::CAUSE() {
    return csr_registers[REG_CSR::CSR_CAUSE];
}

int32_t &Program::PC() {
    return gpr_registers[15];
}

int32_t &Program::SP() {
    return gpr_registers[14];
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
//    push(LR);
//    LR = PC();
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
//    push(LR);
//    LR = PC();
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

int32_t Program::castToSign(int32_t value, uint8_t bites) {
    auto temp = value;
    temp <<= 32 - bites;
    temp >>= 32 - bites;
    return temp;
}

int32_t Program::displacement() {
    return castToSign(currInstr.DISPLACEMENT, 12);
}

