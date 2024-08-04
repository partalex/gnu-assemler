#pragma once

#include "enum.h"

class SectionLink {
public:
    std::vector<uint8_t> _data;
    std::string _name;

    friend std::ostream &operator<<(std::ostream &, const SectionLink &);

    explicit SectionLink(std::string name)
            : _name(std::move(name)) {}

    SectionLink() = default;

    void serialize(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void serializeClean(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void static tableHeader(std::ostream &);

    void reallocateIfNeeded(uint32_t, uint64_t);

    void addToLocCounter(uint32_t);

    int32_t readWord(uint32_t);

    void write(void *, uint32_t, uint32_t);

    void writeAndIncr(void *, uint32_t, uint32_t);

    void writeInstr(void *, uint32_t);

    [[nodiscard]] uint32_t locationCnt() const;

};

