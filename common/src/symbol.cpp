#include "../include/symbol.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

std::istream &operator>>(std::istream &, SCOPE &);

std::ostream &operator<<(std::ostream &, SCOPE);

std::istream &operator>>(std::istream &, SYMBOL &);

std::ostream &operator<<(std::ostream &, SYMBOL);

Symbol::Symbol(std::string name, bool defined, std::string sectionName, SCOPE scope, uint64_t offset,
               SYMBOL symbolType, int32_t size) :
        _name(name),
        _defined(defined),
        _sectionName(sectionName),
        _scope(scope),
        _offset(offset),
        _size(size),
        _symbolType(symbolType) {}


std::string Symbol::serialize() {
    std::stringstream ss;
    ss << std::right <<
       std::setw(15) << "-" <<
       std::setw(15) << _name <<
       std::setw(15) << _defined <<
       std::setw(15) << _sectionName <<
       std::setw(15) << _offset <<
       std::setw(15) << _scope <<
       std::setw(15) << _size <<
       std::setw(15) << _symbolType << "\n";
    return ss.str();
}

Symbol Symbol::deserialize(std::string instr) {
    std::stringstream in;
    in << instr;

    std::string token;
    in >> token;

    std::string name;
    in >> name;

    bool defined;
    in >> defined;

    std::string sectionName;
    in >> sectionName;

    int offset;
    in >> offset;

    SCOPE scope;
    in >> scope;

    int size;
    in >> size;

    SYMBOL symbolType;
    in >> symbolType;

    Symbol sym(name, defined, sectionName, scope, offset, symbolType, size);

    return sym;
}