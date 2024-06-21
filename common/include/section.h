#pragma once

#include "../../common/include/enum.h"
#include "../../common/include/section_link.h"

#include <string>
#include <cstdint>
#include <memory>

class Section {
    uint64_t _locationCounter = 0;
public:

    SectionLink core;

    void addToLocCounter(uint32_t offset);

    [[nodiscard]] uint64_t getLocCounter() const;

    explicit Section(std::string);

    void write(void *, uint32_t, uint32_t);

    void writeInstr(void *, uint32_t);

    Section &operator+=(Section &);

    void serialize(std::ostream &) const;

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    void reallocateMemory(uint32_t, uint64_t);

    uint64_t getSize() const;

};

