#include "../include/symbol_link.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, const SymbolLink &sym) {
    return out << std::left
               << std::setw(20) << sym.name
               << std::setw(11) << (sym.sectionIndex == UNDEFINED ? "UNDEFINED" : std::to_string(sym.sectionIndex))
               << std::setw(11) << (sym.offset == UNDEFINED ? "UNDEFINED" : std::to_string(sym.offset))
               << std::setw(11) << (SCOPE) sym.flags.scope
               << std::setw(11) << (SYMBOL) sym.flags.symbolType
               << std::setw(11) << sym.source
               << "\n";
}

void SymbolLink::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(20) << "Name" <<
        std::setw(11) << "Section" <<
        std::setw(11) << "Offset" <<
        std::setw(11) << "Scope" <<
        std::setw(11) << "Type" <<
        std::setw(11) << "Source"
        << "\n";
}
