#include "../include/section.h"

#include <cstring>
#include <iostream>
#include <utility>

Section::Section(std::string _name) : _core(std::move(_name)) {}

void Section::writeAndIncr(void *src, uint32_t pos, uint32_t length) {
    _core.writeAndIncr(src, pos, length);
}

void Section::writeInstr(void *src, uint32_t pos) {
    _core.writeInstr(src, pos);
}

void Section::serialize(std::ostream &out) const {
    return _core.serialize(out);
}

std::ostream &operator<<(std::ostream &out, Section &sec) {
    return out << sec._core;
}

void Section::tableHeader(std::ostream &out) {
    SectionLink::tableHeader(out);
}

uint32_t Section::getSize() const {
    return _core.locationCnt() + _literalsSection.locationCnt();
}

int32_t Section::readWord(uint32_t offset) {
    return _core.readWord(offset);
}

uint32_t Section::addLiteral(int32_t literal) {
    uint32_t literalOffset = 0;
    if (_literalsMap.find(literal) != _literalsMap.end())
        literalOffset = _literalsMap[literal];
    else {
        literalOffset = _literalsSection._data.size();
        _literalsMap[literal] = literalOffset;
        _literalsSection.writeAndIncr(&literal, literalOffset, 4);
    }
    return literalOffset;
}

void Section::write(void *src, uint32_t pos, uint32_t length) {
    _core.write(src, pos, length);
}

