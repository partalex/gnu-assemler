#pragma once

#include "../../common/include/enum.h"
#include "../../common/include/section_link.h"

#include <string>
#include <cstdint>
#include <memory>

class Section {
public:

    uint32_t locCnt = 0;

    SectionLink core;

    void addToLocCounter(uint32_t offset);

    explicit Section(std::string);

    void write(void *, uint32_t, uint32_t);

    void writeInstr(void *, uint32_t);

    Section &operator+=(Section &);

    void serialize(std::ostream &) const;

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    void reallocateMemory(uint32_t, uint64_t);

    [[nodiscard]] uint32_t getSize() const;

};

