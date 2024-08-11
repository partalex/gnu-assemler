#pragma once

#include "enum.h"

class RelocationLink {
public:
    uint32_t symbolIndex;
    uint32_t sectionIndex;
    int32_t offset;
    RELOCATION type;

    void static tableHeader(std::ostream &);

    static void resolveRelocation(int8_t *, RelocationLink *, void *);

    friend std::ostream &operator<<(std::ostream &, const RelocationLink &);

};


