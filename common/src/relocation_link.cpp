#include "../include/relocation_link.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, const RelocationLink &rel) {
    out << std::left
        << std::setw(11) << (rel.symbolIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.symbolIndex))
        << std::setw(11) << (rel.sectionIndex == UNDEFINED ? "UNDEFINED" : std::to_string(rel.sectionIndex))
        << std::setw(11) << (rel.offset == UNDEFINED ? "UNDEFINED" : std::to_string(rel.offset))
        << std::setw(15) << rel.type
        << "\n";
    return out;
}

void RelocationLink::tableHeader(std::ostream &out) {
    out << std::left
        << std::setw(11) << "Index"
        << std::setw(11) << "Section"
        << std::setw(11) << "Offset"
        << std::setw(15) << "Type"
        << "\n";
}

void RelocationLink::resolveRelocation(int8_t *destAddr, RelocationLink *rel, void *srcAddr) {
    throw std::runtime_error("Not implemented");
//    int32_t temp;
//    switch (rel->core.type) {
//        case R_12b:
//            destAddr = destAddr + rel->offset;
//            temp = (int32_t) (srcAddr - destAddr) + 2;
//            checkDisplacement(temp);
//            writeDisplacement(data.data() + rel->offset, temp);
//            break;
//    }
}
