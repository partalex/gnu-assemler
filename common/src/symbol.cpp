#include "../include/symbol.h"

#include <iostream>
#include <utility>

Symbol::Symbol(std::string name, uint32_t sectionIndex, SCOPE scope, uint32_t offset,
               enum SYMBOL symbolType, SOURCE source, enum DEFINED defined)
        : core(std::move(name), sectionIndex, scope, offset, symbolType, source, defined) {
}

std::ostream &operator<<(std::ostream &out, Symbol &symbol) {
    return out << symbol.core;
}

void Symbol::tableHeader(std::ostream &out) {
    SymbolLink::tableHeader(out);
}
