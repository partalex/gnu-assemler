#include <iostream>
#include "../include/instruction.h"
#include "../include/operand.h"

void Instructions::log() {
    for (auto &entry: _table)
        entry->log();
}

void Instruction::log() {
    std::cout << I::NAMES[static_cast<I::INSTRUCTION>(_byte_1)];
    std::cout << " " << std::to_string(_byte_2);
    std::cout << " " << (std::to_string(_byte_3));
    std::cout << " " << (std::to_string(_byte_4)) << "\n";
}

void Instructions::addInstruction(std::unique_ptr<Instruction> _inst) {
    _table.push_back(std::move(_inst));
}

std::map<I::INSTRUCTION, std::string> I::NAMES = {
        {I::INSTRUCTION::HALT,            "HALT"},
        {I::INSTRUCTION::INT,             "INT"},
        {I::INSTRUCTION::CALL,            "CALL"},
        {I::INSTRUCTION::CALL_MEM,        "CALL_MEM"},

        {I::INSTRUCTION::JMP,             "JMP"},
        {I::INSTRUCTION::BEQ,             "BEQ"},
        {I::INSTRUCTION::BNE,             "BNE"},
        {I::INSTRUCTION::BGT,             "BGT"},
        {I::INSTRUCTION::JMP_MEM,         "JMP_MEM"},
        {I::INSTRUCTION::BEQ_MEM,         "BEQ_MEM"},
        {I::INSTRUCTION::BNE_MEM,         "BNE_MEM"},
        {I::INSTRUCTION::BGT_MEM,         "BGT_MEM"},

        {I::INSTRUCTION::XCHG,            "XCHG"},
        {I::INSTRUCTION::ADD,             "ADD"},
        {I::INSTRUCTION::SUB,             "SUB"},
        {I::INSTRUCTION::MUL,             "MUL"},
        {I::INSTRUCTION::DIV,             "DIV"},

        {I::INSTRUCTION::NOT,             "NOT"},
        {I::INSTRUCTION::AND,             "AND"},
        {I::INSTRUCTION::OR,              "OR"},
        {I::INSTRUCTION::XOR,             "XOR"},
        {I::INSTRUCTION::SHL,             "SHL"},
        {I::INSTRUCTION::SHR,             "SHR"},

        {I::INSTRUCTION::ST,              "ST"},
        {I::INSTRUCTION::ST_IND,          "ST_IND"},
        {I::INSTRUCTION::ST_POST_INC,     "ST_POST_INC (PUSH)"},

        {I::INSTRUCTION::LD_CSR,          "LD_CSR (CSRRD)"},
        {I::INSTRUCTION::LD,              "LD_CSR (IRET)"},
        {I::INSTRUCTION::LD_IND,          "LD_IND"},
        {I::INSTRUCTION::LD_POST_INC,     "LD_POST_INC (POP, RET)"},

        {I::INSTRUCTION::CSR_LD,          "CSRWR"},
        {I::INSTRUCTION::CSR_LD_OR,       "CSR_LD_OR"},
        {I::INSTRUCTION::CSR_LD_IND,      "CSR_LD_IND"},
        {I::INSTRUCTION::CSR_LD_POST_INC, "CSR_LD_POST_INC"},
};

Push_Instr::Push_Instr(uint8_t gpr)
        : Instruction(I::ST_POST_INC, gpr) {}

Pop_Instr::Pop_Instr(uint8_t grp)
        : Instruction(I::LD_POST_INC, grp) {}

Not_Instr::Not_Instr(uint8_t gpr)
        : Instruction(I::NOT, gpr) {}

Int_Instr::Int_Instr(std::unique_ptr<Operand> operand)
        : Instruction(I::INT), _operand(std::move(operand)) {}

Load_Instr::Load_Instr(std::unique_ptr<Operand> operand, uint8_t gpr)
        : Instruction(I::LD, gpr), _operand(std::move(operand)) {}

Csrwr_Instr::Csrwr_Instr(uint8_t gpr, uint8_t csr)
        : Instruction(I::CSR_LD, gpr, csr) {}

Csrrd_Instr::Csrrd_Instr(uint8_t csr, uint8_t gpr)
        : Instruction(I::LD_CSR, csr, gpr) {}

Xchg_Instr::Xchg_Instr(uint8_t regA, uint8_t regB)
        : Instruction(I::XCHG, regA, regB) {}

NoAdr_Instr::NoAdr_Instr(I::INSTRUCTION instruction) : Instruction(instruction) {}

JmpCond_Instr::JmpCond_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Call_Instr::Call_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Jmp_Instr::Jmp_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

TwoReg_Instr::TwoReg_Instr(I::INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS) {}

Store_Instr::Store_Instr(uint8_t gpr, std::unique_ptr<Operand> operand)
        : Instruction(I::ST, gpr), _operand(std::move(operand)) {}
