#include "../include/relocation.h"

#include <iostream>
#include <iomanip>
#include <utility>

std::ostream &operator<<(std::ostream &out, Relocation &rel) {
    return out << std::left <<
               std::setw(15) << rel._symbol << rel._core;
}

void Relocation::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "Symbol";
    RelocationLink::tableHeader(out);
}

Relocation::Relocation(std::string symbol, uint64_t symbolIndex, uint64_t sectionIndex, uint64_t offset, uint64_t size,
                       RELOCATION type)
        : _symbol(std::move(symbol)) {
    _core = {symbolIndex, sectionIndex, offset, size, type};
}

