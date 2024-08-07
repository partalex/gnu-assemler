#include "../include/relocation.h"

#include <iostream>
#include <iomanip>
#include <utility>

std::ostream &operator<<(std::ostream &out, Relocation &rel) {
    return out << std::left <<
               std::setw(20) << rel.symbol <<
               rel.core;
}

void Relocation::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(20) << "Symbol";
    RelocationLink::tableHeader(out);
}

Relocation::Relocation(std::string symbol, uint64_t symbolIndex, uint64_t sectionIndex,
                       uint64_t offset, RELOCATION type) : symbol(std::move(symbol)) {
    core = {symbolIndex, sectionIndex, offset, type};
}

