#pragma once

#include "enum.h"

class SectionLink {
public:
    std::vector<uint8_t> data;
    std::string name;

    friend std::ostream &operator<<(std::ostream &, const SectionLink &);

    explicit SectionLink(std::string name)
            : name(std::move(name)) {}

    SectionLink() = default;

    void serialize(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void serializeClean(std::ostream &, uint64_t startAddress = 0, char fillChar = '.') const;

    void static tableHeader(std::ostream &);
};

