#include "../include/section.h"

#include <cstring>
#include <iostream>
#include <iomanip>


Section::Section(std::string _name, int _size) :
        _size(_size),
        _memory(new u_int8_t[_size]),
        _name(_name) {
}

Section::~Section() {
//    /delete _memory;
}

void Section::Write(void *src, int pos, size_t length) {
    memcpy(_memory + pos, src, length);
}

void Section::WriteZeros(int pos, size_t length) {
    memset(_memory + pos, 0, length);
}

Section &Section::operator+=(Section &other) {
    size_t newSize = _size + other._size;
    uint8_t *newMem = new u_int8_t[newSize];
    memcpy(newMem, _memory, _size);
    memcpy(newMem + _size, other._memory, other._size);
    delete[] _memory;
    delete[] other._memory;
    _memory = newMem;
    _size = newSize;
    return *this;
}


std::string Section::serialize() {
    std::stringstream out;
    const int tokensByLine = 8;
//    out << "Section:" << "\n" ;
    out << _name << " " << std::dec << _size;
    for (int i = 0; i < _size; i++) {
        if (i % tokensByLine == 0)
            out << "\n";
        out << std::right << std::setfill('0') << std::setw(2) << std::hex << (u_int32_t) _memory[i] << " ";
    }

    out << "\n";
    out << ".end" << "\n" << "\n";

    return out.str();
}

Section Section::deserialize(std::string instr) {
    std::stringstream in;
    in << instr;

    std::string _name;
    in >> _name;
    int _size;
    in >> _size;

    Section sec(_name, _size);

    //TODO: really, temp ?
    std::string temptemp;
    u_int32_t temp;
    u_int8_t tempIn;

    for (int i = 0; i < _size; i++) {
        in >> temptemp;

        std::stringstream ss;
        ss << "0x";
        ss << temptemp;

        ss >> std::hex >> temp;

        tempIn = (u_int8_t) temp;

        sec.Write(&tempIn, i, 1);
    }

    return sec;

}

