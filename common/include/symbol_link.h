#pragma once

#include "enum.h"

typedef struct {
    uint8_t symbolType: 2, scope: 1;
} SymbolLinkFlags;


class SymbolLink {
public:
    std::string name;
    uint64_t sectionIndex;
    uint64_t offset;
    SymbolLinkFlags flags;
    SOURCE source;

    void static tableHeader(std::ostream &);

    explicit SymbolLink(std::string name, uint64_t sectionIndex, SCOPE scope, uint64_t offset, SYMBOL symbolType,
                        SOURCE source)
            : name(std::move(name)), sectionIndex(sectionIndex), offset(offset), source(source) {
        flags.scope = scope;
        flags.symbolType = symbolType;
    }

    SymbolLink() = default;

    friend std::ostream &operator<<(std::ostream &, const SymbolLink &);
};
