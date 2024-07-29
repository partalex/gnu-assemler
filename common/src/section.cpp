#include "../include/section.h"

#include <cstring>
#include <iostream>
#include <utility>

Section::Section(std::string _name)
        : core(std::move(_name)) {}


void Section::write(void *src, uint32_t pos, uint32_t length) {
    reallocateMemory(pos, length);
    std::memcpy(core.data.data() + pos, src, length);
}


void Section::writeAndIncr(void *src, uint32_t pos, uint32_t length) {
    write(src, pos, length);
    addToLocCounter(length);
}

void Section::writeInstr(void *src, uint32_t pos) {
    writeAndIncr(src, pos, 4);
}

Section &Section::operator+=(Section &other) {
    core.data.resize(core.data.size() + other.core.data.size());
    std::memcpy(core.data.data() + core.data.size(), other.core.data.data(), other.core.data.size());
    return *this;
}

void Section::serialize(std::ostream &out) const {
    return core.serialize(out);
}

std::ostream &operator<<(std::ostream &out, Section &sec) {
    return out << sec.core;
}

void Section::tableHeader(std::ostream &out) {
    SectionLink::tableHeader(out);
}

void Section::reallocateMemory(uint32_t pos, uint64_t length) {
    if (pos + length <= core.data.size())
        return;
    auto newSize = pos + length;
    core.data.resize(newSize);
}

void Section::addToLocCounter(uint32_t offset) {
    reallocateMemory(locCnt, offset);
    locCnt += offset;
}

uint32_t Section::getSize() const {
    return core.data.size();
}

int32_t Section::readWord(uint32_t offset) {
    int32_t value = 0;
    std::memcpy(&value, core.data.data() + offset, 4);
    return value;
}
