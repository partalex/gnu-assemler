#include "../include/relocation_link.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, const RelocationLink &rel) {
    out << std::left
        << std::setw(11) << (rel.symbolIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.symbolIndex))
        << std::setw(11) << (rel.sectionIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.sectionIndex))
        << std::setw(11) << (rel.offset == UNDEFINED ? "UNDEFINED" : std::to_string(rel.offset))
        << std::setw(11) << rel.size
        << std::setw(15) << rel.type
        << "\n";
    return out;
}

void RelocationLink::tableHeader(std::ostream &out) {
    out << std::left
        << std::setw(11) << "Symbol"
        << std::setw(11) << "Section"
        << std::setw(11) << "Offset"
        << std::setw(11) << "Size"
        << std::setw(15) << "Type"
        << "\n";
}