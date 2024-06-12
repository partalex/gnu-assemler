#include "../include/instruction.h"
#include "../include/operand.h"

#include <iostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &out, Instruction &instr) {
    out << std::left <<
        std::setw(15) << std::hex << (short) instr._bytes._byte_3 <<
        std::setw(15) << std::hex << (short) instr._bytes._byte_2 <<
        std::setw(15) << std::hex << (short) instr._bytes._byte_1 <<
        std::setw(15) << std::hex << (short) instr._bytes._byte_0 << "\n";
//    out << "Instruction: " << instr.instructionSymbol << "\n";
//    out << "Condition: " << instr.instructionCondition << "\n";
//    out << "setflags: " << instr.setFlags << "\n";
//    for (auto &temp: instr.parameters)
//        out << "\t" << temp << "\n";
//    out << "\tcode:" << std::hex << instr.instrCode.binaryCode << "\n";
//    out << "\n";
    return out;
}

void Instruction::setDisplacement(int16_t offset) {
    if (offset < -2048 || offset > 2047)
        throw std::runtime_error("Displacement out of range.");
    _bytes._byte_2 |= (offset & 0xF00) >> 8;
    _bytes._byte_3 |= offset & 0xFF;
}

Instruction::Instruction(enum INSTRUCTION instr, uint8_t regA, uint8_t regB, uint8_t regC) {
    setInstr(instr);
    setRegA(regA);
    setRegB(regB);
    setRegC(regC);
}

void Instruction::setInstr(uint8_t instr) {
    _bytes._byte_0 = instr;
}

void Instruction::setRegA(uint8_t regA) {
    _bytes._byte_1 |= regA << 4;
}

void Instruction::setRegB(uint8_t regB) {
    _bytes._byte_2 |= regB;
}

void Instruction::setRegC(uint8_t regC) {
    _bytes._byte_2 |= regC << 4;
}

void Instructions::addInstruction(std::unique_ptr<Instruction> _inst) {
    _table.push_back(std::move(_inst));
}

Push_Instr::Push_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::ST_POST_INC, gpr) {}

Pop_Instr::Pop_Instr(uint8_t grp)
        : Instruction(INSTRUCTION::LD_POST_INC, grp) {}

Not_Instr::Not_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::NOT, gpr) {}

Int_Instr::Int_Instr(std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::INT), _operand(std::move(operand)) {}

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

NoAdr_Instr::NoAdr_Instr(enum INSTRUCTION instruction) :
        Instruction(instruction) {}

JmpCond_Instr::JmpCond_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Call_Instr::Call_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Jmp_Instr::Jmp_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

TwoReg_Instr::TwoReg_Instr(enum INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS) {}

Store_Instr::Store_Instr(uint8_t gpr, std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::ST, gpr), _operand(std::move(operand)) {}

Instruction Instruction::deserialize(uint32_t instructionCode) {
    Instruction instruction(INSTRUCTION::HALT);
    // TODO
//    Instruction instruction(INSTRUCTION::HALT, NO_CONDITION, false);
//    instruction.instrCode.binaryCode = instructionCode;
//    instruction.instructionSymbol = (InstructionSymbol) instruction.instrCode.instruction.instr;
//    instruction.instructionCondition = (InstructionCondition) instruction.instrCode.instruction.cond;
//    instruction.setFlags = instruction.instrCode.instruction.flag != 0;
    return instruction;
}

void Instruction::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "BYTE[3]" <<
        std::setw(15) << "BYTE[2]" <<
        std::setw(15) << "BYTE[1]" <<
        std::setw(15) << "BYTE[0]" << "\n";
}
