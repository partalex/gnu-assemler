#pragma once

#include <string>
#include <memory>

struct SymbolList {
    std::string _symbol;
    SymbolList *_next = nullptr;

    explicit SymbolList(const std::string &);

    SymbolList(const std::string &, SymbolList *_next);

    ~SymbolList() { delete _next; }

    void log();
};

