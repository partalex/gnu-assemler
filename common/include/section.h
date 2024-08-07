#pragma once

#include "../../common/include/enum.h"
#include "../../common/include/section_link.h"

#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

class Relocation;

class Section {
public:

    SectionLink core;
    SectionLink literalsSection;
    std::unordered_map<int32_t, uint32_t> literalsMap;
    std::unordered_map<std::string, uint32_t> labelsMap;

    uint32_t addLiteral(int32_t);

    uint32_t addLabel(const std::string &);

    explicit Section(std::string);

    int32_t readWord(uint32_t offset);

    void appendInstr(void *);

    void serialize(std::ostream &) const;

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    [[nodiscard]] uint32_t getSize() const;

    void appendLiterals();

};

