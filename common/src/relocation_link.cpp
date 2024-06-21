#include "../include/relocation_link.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, const RelocationLink &rel) {
    out << std::left <<
        std::setw(15) << (rel.symbolIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.symbolIndex)) <<
        std::setw(15) << (rel.sectionIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.sectionIndex)) <<
        std::setw(15) << (rel.offset == UNDEFINED ? "UNDEFINED" : std::to_string(rel.offset)) <<
        std::setw(15) << rel.size <<
        std::setw(15) << rel.type << "\n";
    return out;
}

void RelocationLink::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "SymbolIndex" <<
        std::setw(15) << "SectionIndex" <<
        std::setw(15) << "Offset" <<
        std::setw(15) << "Size" <<
        std::setw(15) << "Type" << "\n";
}