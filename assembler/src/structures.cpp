#include "structures.hpp"
#include "log.hpp"

std::string Csr::CSR[] = {"status", "handler", "cause"};

SymbolList::SymbolList(const std::string &str, SymbolList *next) {
    _symbol = str;
    _next = next;
}

SymbolList::SymbolList(const std::string &str) {
    _symbol = str;
}

void SymbolList::log() {
#ifdef DO_DEBUG
    SymbolList *current = this;
    while (current != nullptr) {
        Log::STRING(current->_symbol);
        current = current->_next;
        if (current)
            Log::STRING(", ");
    }
    Log::STRING_LN("");
#endif
}

void Operand::logOne() {
#ifdef DO_DEBUG
    if (_selector & 0b10000000)
        Log::STRING(std::to_string(_gpr1));
    if (_selector & 0b01000000)
        Log::STRING(std::to_string(_literal));
    if (_selector & 0b00100000)
        Log::STRING(_symbol);
    if (_selector & 0b00010000)
        Log::STRING(std::to_string(_csr));
    if (_selector & 0b00001000)
        Log::STRING(std::to_string(_gpr2));
#endif
}

void Operand::log() {
#ifdef DO_DEBUG
    Operand *current = this;
    while (current != nullptr) {
        current->log();
        current = current->_next;
        if (current)
            Log::STRING_LN("");
    }
#endif
}

std::map<I::INSTRUCTION, std::string> I::NAMES = {
        {I::INSTRUCTION::HALT,  "HALT"},
        {I::INSTRUCTION::IRET,  "IRET or POP"},
        {I::INSTRUCTION::RET,   "RET"},
        {I::INSTRUCTION::INT,   "INT"},
        {I::INSTRUCTION::CALL,  "CALL"},
        {I::INSTRUCTION::JMP,   "JMP"},
        {I::INSTRUCTION::POP,   "POP"},
        {I::INSTRUCTION::PUSH,  "PUSH"},
        {I::INSTRUCTION::NOT,   "NOT"},
        {I::INSTRUCTION::XCHG,  "XCHG"},
        {I::INSTRUCTION::ADD,   "ADD"},
        {I::INSTRUCTION::SUB,   "SUB"},
        {I::INSTRUCTION::MUL,   "MUL"},
        {I::INSTRUCTION::DIV,   "DIV"},
        {I::INSTRUCTION::AND,   "AND"},
        {I::INSTRUCTION::OR,    "OR"},
        {I::INSTRUCTION::XOR,   "XOR"},
        {I::INSTRUCTION::SHL,   "SHL"},
        {I::INSTRUCTION::SHR,   "SHR"},
        {I::INSTRUCTION::CSRRD, "CSRRD"},
        {I::INSTRUCTION::CSRWR, "CSRWR"},
        {I::INSTRUCTION::LD,    "LD or CSRRD"},
        {I::INSTRUCTION::ST,    "ST"}
};
