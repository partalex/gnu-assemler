#include "structures.hpp"
#include "log.hpp"

std::string Csr::CSR[] = {"status", "handler", "cause"};

Operand::Operand(const std::string &value) : _value(value) {
#ifdef DO_DEBUG
    Log::STRING_LN(value);
#endif
}


SymbolList::SymbolList(const std::string &, SymbolList *_next) {
#ifdef DO_DEBUG
    Log::STRING_LN("SymbolList: " + _symbol);
#endif
}

SymbolList::SymbolList(const std::string &) {
#ifdef DO_DEBUG
    Log::STRING_LN("SymbolList: " + _symbol);
#endif
}
