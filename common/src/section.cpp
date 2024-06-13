#include "../include/section.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <utility>

const uint64_t Section::MIN_SIZE = 64;
const uint8_t Section::MULTIPLIER = 2;

Section::Section(std::string _name) :
        _name(std::move(_name)) {
    _memory = std::make_unique<uint8_t[]>(MIN_SIZE);
}

void Section::write(void *src, uint32_t pos, uint32_t length) {
    if (pos + length > _size)
        reallocateMemory(_size ? _size * MULTIPLIER : MIN_SIZE);
    std::memcpy(_memory.get() + pos, src, length);
}

void Section::writeInstr(void *src, uint32_t pos) {
    write(src, pos, 4);
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

void Section::serialize(std::ostream &out) const {
    const int tokensByLine = 16;
    out << _name << " " << std::dec << _size;
    for (auto i = 0; i < _size; ++i) {
        if (i % tokensByLine == 0)
            out << "\n" << std::setw(8) << std::setfill('0') << std::hex << i << "   ";
        out << std::right << std::setfill('0') << std::setw(2) << std::hex << (uint32_t) _memory[i] << " ";
        if (i % 8 == 7)
            out << "  ";
        if (i % tokensByLine == tokensByLine - 1) {
            out << "|";
            for (auto j = i - tokensByLine + 1; j <= i; ++j)
                if (_memory[j] >= 32 && _memory[j] <= 126)
                    out << _memory[j];
                else
                    out << ".";
            out << "|";
        }
    }
    auto remaining = _size % tokensByLine;
    if (remaining != 0) {
        for (int i = 0; i < tokensByLine - remaining; i++)
            out << "   ";
        out << " |";
        for (auto i = _size - remaining; i < _size; ++i)
            if (_memory[i] >= 32 && _memory[i] <= 126)
                out << _memory[i];
            else
                out << ".";
        out << "|";
    }
    out << "\n" << ".end" << "\n" << "\n";
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
    if (_locationCounter + offset > _size)
        reallocateMemory(_size * MULTIPLIER);
    _locationCounter += offset;
}

uint64_t Section::getLocCounter() const {
    return _locationCounter;
}
