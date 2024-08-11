#include "../include/symbol_link.h"

#include <iomanip>

SymbolLink::SymbolLink(std::string name, uint32_t sectionIndex, SCOPE scope, uint32_t offset, SYMBOL symbolType,
                       SOURCE source, enum DEFINED defined)
        : name(std::move(name)), sectionIndex(sectionIndex), offset(offset) {
    flags.scope = scope;
    flags.symbolType = symbolType;
    flags.source = source;
    flags.defined = defined;
}

std::ostream &operator<<(std::ostream &out, const SymbolLink &sym) {
    return out << std::left
               << std::setw(20) << sym.name
               << std::setw(11) << (MARKER) (sym.sectionIndex)
               << std::setw(11) << (MARKER) (sym.offset)
               << std::setw(11) << (SCOPE) (sym.flags.scope)
               << std::setw(11) << (SYMBOL) (sym.flags.symbolType)
               << std::setw(11) << (SOURCE) (sym.flags.source)
               << std::setw(11) << (enum DEFINED) (sym.flags.defined)
               << "\n";
}

void SymbolLink::tableHeader(std::ostream &out) {
    out << std::left
        << std::setw(20) << "Name"
        << std::setw(11) << "Section"
        << std::setw(11) << "Offset"
        << std::setw(11) << "Scope"
        << std::setw(11) << "Type"
        << std::setw(11) << "Source"
        << std::setw(11) << "Defined"
        << "\n";
}


