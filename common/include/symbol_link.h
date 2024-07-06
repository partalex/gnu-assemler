#pragma once

#include "enum.h"

typedef struct {
    uint8_t defined: 1, source: 1, symbolType: 2, scope: 1;
} SymbolLinkFlags;


class SymbolLink {
public:
    std::string name;
    uint32_t sectionIndex;
    uint32_t offset;
    SymbolLinkFlags flags;

    void static tableHeader(std::ostream &);

    explicit SymbolLink(std::string, uint32_t, SCOPE, uint32_t, SYMBOL,
                        SOURCE, enum DEFINED);

    SymbolLink() = default;

    friend std::ostream &operator<<(std::ostream &, const SymbolLink &);

};
