#include "../include/symbol_link.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, const SymbolLink &sym) {
    return out << std::left <<
               std::setw(15) << sym.name <<
               std::setw(15) << (sym.sectionIndex == UNDEFINED ? "UNDEFINED" : std::to_string(sym.sectionIndex)) <<
               std::setw(15) << (sym.offset == UNDEFINED ? "UNDEFINED" : std::to_string(sym.offset)) <<
               std::setw(15) << (SCOPE) sym.flags.scope <<
               std::setw(15) << (SYMBOL) sym.flags.symbolType << "\n";
}

void SymbolLink::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "Name" <<
        std::setw(15) << "SectionIndex" <<
        std::setw(15) << "Offset" <<
        std::setw(15) << "Scope" <<
        std::setw(15) << "SymbolType" << "\n";
}
