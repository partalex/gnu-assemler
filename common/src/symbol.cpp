#include "../include/symbol.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

Symbol::Symbol(std::string name, bool defined, std::string sectionName, SCOPE scope, uint64_t offset,
               enum SYMBOL symbolType, int32_t size) :
        _name(name),
        _defined(defined),
        _sectionName(sectionName),
        _scope(scope),
        _offset(offset),
        _size(size),
        _symbolType(symbolType) {}

std::istream &operator>>(std::istream &in, Symbol &scope) {
    in >> scope._name;
    in >> scope._defined;
    in >> scope._sectionName;
    in >> scope._offset;
    in >> scope._scope;
    in >> scope._size;
    in >> scope._symbolType;
    return in;
}

std::ostream &operator<<(std::ostream &out, Symbol &sym) {
    out << "Symbol: " << sym._name << "\n";
    out << "\tDefined:\t" << sym._defined << "\n";
    out << "\tSectionName:\t" << sym._sectionName << "\n";
    out << "\tOffset:\t" << sym._offset << "\n";
    out << "\tType:\t" << sym._scope << "\n" << "\n";
    out << "\tSize:\t" << sym._size << "\n" << "\n";
    return out;
}

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

    enum SYMBOL symbolType;
    in >> symbolType;

    Symbol sym(name, defined, sectionName, scope, offset, symbolType, size);

    return sym;
}