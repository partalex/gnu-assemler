#include "instruction.hpp"

void Instructions::log() {
    for (auto &entry: _table)
        entry->log();
}

void Instruction::log() {
    Log::STRING(I::NAMES[static_cast<I::INSTRUCTION>(_byte_1)]);
    Log::STRING(" ");
    Log::STRING(std::to_string(_byte_2));
    Log::STRING(" ");
    Log::STRING(std::to_string(_byte_3));
    Log::STRING(" ");
    Log::STRING_LN(std::to_string(_byte_4));
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