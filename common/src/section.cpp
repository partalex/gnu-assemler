#include "../include/section.h"
#include "../include/relocation.h"

#include <iostream>
#include <utility>

Section::Section(std::string _name) : core(std::move(_name)) {}

void Section::appendInstr(void *src) {
    core.append(src, 4);
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

uint32_t Section::getSize() const {
    return core.locationCnt() + literalsSection.locationCnt();
}

int32_t Section::readWord(uint32_t offset) {
    return core.readWord(offset);
}

uint32_t Section::addLiteral(int32_t literal) {
    if (literalsMap.find(literal) != literalsMap.end())
        return literalsMap[literal];
    literalsMap[literal] = literalsSection.data.size();
    literalsSection.append(&literal, 4);
    return literalsMap[literal];
}

void Section::appendLiterals() {
    core.append(literalsSection.data.data(), literalsSection.data.size());
}

uint32_t Section::addLabel(const std::string &label) {
    if (labelsMap.find(label) != labelsMap.end())
        return labelsMap[label];
    int32_t fill = 0;
    labelsMap[label] = literalsSection.locationCnt();
    literalsSection.append(&fill, 4);
    return labelsMap[label];
}



