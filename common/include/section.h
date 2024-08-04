#pragma once

#include "../../common/include/enum.h"
#include "../../common/include/section_link.h"

#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

class Section {
public:

    uint32_t _locCnt = 0;
    SectionLink _core;
    SectionLink _literalsSection;
    std::unordered_map<int32_t, uint32_t> _literalsMap;

    // return offset of literal
    uint32_t addLiteral(int32_t);

    explicit Section(std::string);

    void writeAndIncr(void *, uint32_t, uint32_t);

    void write(void *, uint32_t, uint32_t);

    int32_t readWord(uint32_t offset);

    void writeInstr(void *, uint32_t);

    void serialize(std::ostream &) const;

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    [[nodiscard]] uint32_t getSize() const;

};

