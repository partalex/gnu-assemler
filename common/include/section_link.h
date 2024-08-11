#pragma once

#include "enum.h"

class SectionLink {
public:
    std::vector<uint8_t> data;
    std::string name;

    friend std::ostream &operator<<(std::ostream &, const SectionLink &);

    SectionLink &operator+=(const SectionLink &);

    explicit SectionLink(std::string name)
            : name(std::move(name)) {}

    SectionLink() = default;

    void serialize(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void serializeClean(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void static tableHeader(std::ostream &);

    void reallocateIfNeeded(uint32_t, uint32_t);

    [[nodiscard]] bool checkSize(uint32_t, uint32_t);

    void addToLocCounter(uint32_t);

    int32_t readWord(uint32_t);

    void write(const void *, uint32_t, uint32_t);

    void writeWord(void *, uint32_t);

    void fixWord(void *, uint32_t);

    void append(const void *, uint32_t);

    [[nodiscard]] uint32_t locationCnt() const;

};

