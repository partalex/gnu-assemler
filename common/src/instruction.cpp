#include "../include/operand.h"
#include "../include/program.h"
#include "../include/instruction.h"


#include <iostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &out, Instruction &instr) {
    out << std::left <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_3 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_2 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_1 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_0 << "\n";
//    out << "Instruction: " << instr.instructionSymbol << "\n";
//    out << "Condition: " << instr.instructionCondition << "\n";
//    out << "setflags: " << instr.setFlags << "\n";
//    for (auto &temp: instr.parameters)
//        out << "\t" << temp << "\n";
//    out << "\tcode:" << std::hex << instr.instrCode.binaryCode << "\n";
//    out << "\n";
    return out;
}

void Instruction::setDisplacement(int32_t offset) {
    if (!fitIn12Bits(offset))
        throw std::runtime_error("Displacement out of range.");
    bytes.DISPLACEMENT = offset;
}

Instruction::Instruction(enum INSTRUCTION instr, uint8_t regA, uint8_t regB, uint8_t regC, int32_t disp) {
    setInstr(instr);
    setRegA(regA);
    setRegB(regB);
    setRegC(regC);
    setDisplacement(disp);
}

void Instruction::setInstr(uint8_t instr) {
    bytes.byte_0 = instr;
}

void Instruction::setMode(uint8_t mode) {
    bytes.MODE = mode;
}

void Instruction::andMode(uint8_t value) {
    bytes.MODE |= value;
}
void Instruction::orMode(uint8_t value) {
    bytes.MODE |= value;
}

void Instruction::setRegA(uint8_t regA) {
    bytes.REG_A = regA;
}

void Instruction::setRegB(uint8_t regB) {
    bytes.REG_B = regB;
}

void Instruction::setRegC(uint8_t regC) {
    bytes.REG_C = regC;
}

Push_Instr::Push_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::ST_POST_INC, REG_SP, 0, gpr, -4) {
}

Pop_Instr::Pop_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::LD_POST_INC, gpr, REG_SP, 0, 4) {
}

Not_Instr::Not_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::NOT, gpr) {}

Int_Instr::Int_Instr()
        : Instruction(INSTRUCTION::INT) {}

Load_Instr::Load_Instr(std::unique_ptr<Operand> operand, uint8_t gpr)
        : Instruction(INSTRUCTION::LD, gpr), _operand(std::move(operand)) {
}

Load_Instr::Load_Instr(uint8_t gprD, uint8_t gprS, int16_t offset)
        : Instruction(INSTRUCTION::LD, gprD, gprS) {
    setDisplacement(offset);
}

Csrwr_Instr::Csrwr_Instr(uint8_t gpr, uint8_t csr)
        : Instruction(INSTRUCTION::CSR_LD, gpr, csr) {}

Csrrd_Instr::Csrrd_Instr(uint8_t csr, uint8_t gpr)
        : Instruction(INSTRUCTION::LD_CSR, csr, gpr) {}

Xchg_Instr::Xchg_Instr(uint8_t regA, uint8_t regB)
        : Instruction(INSTRUCTION::XCHG, regA, regB) {}

JmpCond_Instr::JmpCond_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {
    setRegA(REG_PC);
    auto *try1 = dynamic_cast<GprGprIdent *>(_operand.get());
    auto *try2 = dynamic_cast<GprGprLiteral *>(_operand.get());
    if (try1) {
        setRegB(try1->getGpr1());
        setRegC(try1->getGpr2());
    } else {
        setRegB(try2->getGpr1());
        setRegC(try2->getGpr2());
    }
}

Call_Instr::Call_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {
    setRegA(REG_PC);
    setRegB(GPR_R0);
}

Jmp_Instr::Jmp_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {
    setRegA(15);
}

TwoReg_Instr::TwoReg_Instr(enum INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS, regS) {}

Store_Instr::Store_Instr(uint8_t gpr, std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::ST), _operand(std::move(operand)) {
    setRegC(gpr);
}

void Instruction::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "BYTE[3]" <<
        std::setw(15) << "BYTE[2]" <<
        std::setw(15) << "BYTE[1]" <<
        std::setw(15) << "BYTE[0]" << "\n";
}

Instruction::Instruction(uint32_t value) {
    bytes.value = value;
}

std::ostream &Instruction::logExecute(std::ostream &out) const {
    return out << std::hex << std::setfill('0') << std::setw(8) << bytes.byte_0 << " "
               << std::setw(8) << bytes.byte_1 << " "
               << std::setw(8) << bytes.byte_2 << " "
               << std::setw(8) << bytes.byte_3 << " ";
}

bool Instruction::fitIn12Bits(int32_t value) {
    return value >= -2048 && value <= 2047;
}

