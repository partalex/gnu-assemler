#include "../include/symbol.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <utility>

Symbol::Symbol(std::string name, bool defined, std::uint32_t sectionIndex, SCOPE scope, uint32_t offset,
               enum SYMBOL symbolType, int32_t size) :
        _name(std::move(name)),
        _defined(defined),
        _sectionIndex(sectionIndex),
        _scope(scope),
        _offset(offset),
        _size(size),
        _symbolType(symbolType) {}

std::istream &operator>>(std::istream &in, Symbol &symbol) {
    in >> symbol._name;
    in >> symbol._defined;
    in >> symbol._sectionIndex;
    in >> symbol._offset;
    in >> symbol._scope;
    in >> symbol._size;
    in >> symbol._symbolType;
    return in;
}

std::ostream &operator<<(std::ostream &out, Symbol &symbol) {
    out << std::left <<
        std::setw(15) << symbol._name <<
        std::setw(15) << symbol._defined <<
        std::setw(15) << symbol._sectionIndex <<
        std::setw(15) << symbol._offset <<
        std::setw(15) << symbol._scope <<
        std::setw(15) << symbol._size <<
        std::setw(15) << symbol._symbolType << "\n";
    return out;
}

std::string Symbol::serialize() const {
    std::stringstream ss;
    ss << std::right <<
       std::setw(15) << "-" <<
       std::setw(15) << _name <<
       std::setw(15) << _defined <<
       std::setw(15) << _sectionIndex <<
       std::setw(15) << _offset <<
       std::setw(15) << _scope <<
       std::setw(15) << _size <<
       std::setw(15) << _symbolType << "\n";
    return ss.str();
}

Symbol Symbol::deserialize(const std::string &instr) {
    std::stringstream in;
    in << instr;

    std::string token;
    in >> token;

    std::string name;
    in >> name;

    bool defined;
    in >> defined;

    uint32_t sectionIndex;
    in >> sectionIndex;

    int offset;
    in >> offset;

    SCOPE scope;
    in >> scope;

    int size;
    in >> size;

    enum SYMBOL symbolType;
    in >> symbolType;

    Symbol sym(name, defined, sectionIndex, scope, offset, symbolType, size);

    return sym;
}

void Symbol::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "Name" <<
        std::setw(15) << "Defined" <<
        std::setw(15) << "SectionIndex" <<
        std::setw(15) << "Offset" <<
        std::setw(15) << "Scope" <<
        std::setw(15) << "Size" <<
        std::setw(15) << "SymbolType" << "\n";
}
