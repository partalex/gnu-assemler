#include "../include/relocation.h"

#include <iostream>
#include <iomanip>

std::istream &operator>>(std::istream &in, RELOCATION &rel) {
    std::string token;
    in >> token;
    if (token == "R_386_32")
        rel = RELOCATION::R_386_32;
    if (token == "R_386_PC32")
        rel = RELOCATION::R_386_PC32;
    return in;
}

std::ostream &operator<<(std::ostream &out, Relocation &rel) {
    out << "Relocation: " << "\n";
    out << "\tSymbol:\t" << rel._symbolName << "\n";
    out << "\tSection:\t" << rel._section << "\n";
    out << "\tOffset:\t" << rel._offset << "\n";
    out << "\tRelocationType:\t" << rel._relocationType << "\n" << "\n";
    return out;
}

Relocation::Relocation(std::string symbolName, std::string section, u_int32_t offset, RELOCATION relocationType)
        : _section(section), _offset(offset), _relocationType(relocationType), _symbolName(symbolName) {
}

std::string Relocation::serialize() {
    std::stringstream out;

    out << std::right <<
        std::setw(15) << "-" <<
        std::setw(15) << _symbolName <<
        std::setw(15) << _section <<
        std::setw(15) << _offset <<
        std::setw(15) << _relocationType << "\n";

    return out.str();
}

Relocation Relocation::deserialize(std::string instr) {
    std::stringstream in;
    in << instr;

    std::string token;
    in >> token;

    std::string symbolName;
    in >> symbolName;

    std::string section;
    in >> section;

    u_int32_t offset;
    in >> offset;

    RELOCATION relType;
    in >> relType;

    Relocation rel(symbolName, section, offset, relType);

    return rel;
}