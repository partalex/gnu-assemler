#include "../include/section.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <utility>

const uint64_t Section::MIN_SIZE = 128;
const uint8_t Section::MULTIPLIER = 2;

Section::Section(std::string _name) :
        _name(std::move(_name)) {
    _memory = std::make_unique<uint8_t[]>(MIN_SIZE);
}

void Section::write(void *src, uint32_t pos, uint64_t length) {
    if (pos + length > _size)
        reallocateMemory(_size ? _size * MULTIPLIER : MIN_SIZE);
    std::memcpy(_memory.get() + pos, src, length);
}

void Section::writeZeros(uint32_t pos, uint64_t length) {
    if (pos + length > _size)
        reallocateMemory(_size * MULTIPLIER);
    std::memset(_memory.get() + pos, 0, length);
}

Section &Section::operator+=(Section &other) {
    uint64_t newSize = _size + other._size;
    auto newMem = std::make_unique<uint8_t[]>(newSize);
    std::memcpy(newMem.get(), _memory.get(), _size);
    std::memcpy(newMem.get() + _size, other._memory.get(), other._size);
    _memory = std::move(newMem);
    _size = newSize;
    return *this;
}

std::string Section::serialize() const {
    std::stringstream out;
    const int tokensByLine = 8;
    out << _name << " " << std::dec << _size;
    for (int i = 0; i < _size; i++) {
        if (i % tokensByLine == 0)
            out << "\n";
        out << std::right << std::setfill('0') << std::setw(2) << std::hex << (uint32_t) _memory[i] << " ";
    }
    out << "\n";
    out << ".end" << "\n" << "\n";
    return out.str();
}

Section Section::deserialize(const std::string &instr) {
    std::stringstream in;
    in << instr;
    std::string _name;
    in >> _name;
    int _size;
    in >> _size;
    Section sec(_name);
    sec.reallocateMemory(_size);
    std::string tempS;
    uint32_t temp;
    uint8_t tempIn;
    for (int i = 0; i < _size; ++i) {
        in >> tempS;
        std::stringstream ss;
        ss << "0x";
        ss << tempS;
        ss >> std::hex >> temp;
        tempIn = (uint8_t) temp;
        sec.write(&tempIn, i, 1);
    }
    return sec;
}

std::ostream &operator<<(std::ostream &out, Section &sec) {
    out << std::left <<
        std::setw(15) << sec._name <<
        std::setw(15) << sec._size <<
        std::setw(15) << sec._locationCounter << "\n";
    return out;
// TODO
//    const int tokensByLine = 8;
//    for (int i = 0; i < sec._size; ++i) {
//        if (i % tokensByLine == 0)
//            out << "\n";
//        out << std::setfill('0') << std::setw(2) << std::hex
//            << (uint32_t) sec._memory[i] << " ";
//    }
//    out << "\n";
}

void Section::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "Name" <<
        std::setw(15) << "Size" <<
        std::setw(15) << "LocCounter" << "\n";
}

void Section::reallocateMemory(uint64_t size) {
    auto newMem = std::make_unique<uint8_t[]>(size);
    if (_size)
        std::memcpy(newMem.get(), _memory.get(), _size);
    _memory = std::move(newMem);
    _size = size;
}

void Section::addToLocCounter(uint32_t offset) {
    _locationCounter += offset;
}

uint64_t Section::getLocCounter() const {
    return _locationCounter;
}
