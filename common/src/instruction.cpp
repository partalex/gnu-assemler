#include "../include/instruction.h"
#include "../include/operand.h"


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
        : Instruction(INSTRUCTION::LD, gpr), _operand(std::move(operand)) {}

Csrwr_Instr::Csrwr_Instr(uint8_t gpr, uint8_t csr)
        : Instruction(INSTRUCTION::CSR_LD, gpr, csr) {}

Csrrd_Instr::Csrrd_Instr(uint8_t csr, uint8_t gpr)
        : Instruction(INSTRUCTION::LD_CSR, csr, gpr) {}

Xchg_Instr::Xchg_Instr(uint8_t regA, uint8_t regB)
        : Instruction(INSTRUCTION::XCHG, regA, regB) {}

NoAdr_Instr::NoAdr_Instr(INSTRUCTION instruction) :
        Instruction(instruction) {}

JmpCond_Instr::JmpCond_Instr(INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Call_Instr::Call_Instr(INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Jmp_Instr::Jmp_Instr(INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

TwoReg_Instr::TwoReg_Instr(INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS) {}

Store_Instr::Store_Instr(uint8_t gpr, std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::ST, gpr), _operand(std::move(operand)) {}
