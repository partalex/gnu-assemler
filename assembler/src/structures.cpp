#include "structures.hpp"
#include "log.hpp"
#include <algorithm>

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
#endif
}

void Operand::log() {
#ifdef DO_DEBUG
    Operand *current = this;
    while (current != nullptr) {
        current->logOne();
        current = current->_next;
        if (current)
            Log::STRING_LN("");
    }
#endif
}

bool SymbolTable::hasUnresolvedSymbols() {
    return false;
}

void SymbolTable::addSymbol(const SymbolTableEntry &entry) {
    _table.push_back(entry);
}

bool SymbolTable::checkSymbol(BIND bind, ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, bind, type](const SymbolTableEntry &entry) {
                return entry._name == name && entry._bind == bind && entry._type == type;
            });
    return res != _table.end();
}

SymbolTableEntry *SymbolTable::getSymbol(BIND bind, ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, bind, type](const SymbolTableEntry &entry) {
                return entry._name == name && entry._bind == bind && entry._type == type;
            });
    if (res != _table.end())
        return &(*res);
    return nullptr;
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
