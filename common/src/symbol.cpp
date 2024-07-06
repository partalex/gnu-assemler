#include "../include/symbol.h"

#include <iostream>
#include <utility>

Symbol::Symbol(std::string name, uint64_t sectionIndex, SCOPE scope, uint64_t offset,
               enum SYMBOL symbolType, SOURCE source)
        : core(std::move(name), sectionIndex, scope, offset, symbolType, source) {
}

std::ostream &operator<<(std::ostream &out, Symbol &symbol) {
    return out << symbol.core;
}

void Symbol::tableHeader(std::ostream &out) {
    SymbolLink::tableHeader(out);
}
