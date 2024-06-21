#pragma once

#include "enum.h"

class RelocationLink {
public:
    uint64_t symbolIndex;
    uint64_t sectionIndex;
    uint64_t offset;
    uint64_t size;
    RELOCATION type;

    void static tableHeader(std::ostream &);

    friend std::ostream &operator<<(std::ostream &, const RelocationLink &);

};


