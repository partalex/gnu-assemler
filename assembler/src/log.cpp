#include <iostream>
#include "log.hpp"

void Log::STRING(std::string const& str) {
    std::cerr << str;
}

void Log::STRING_LN(std::string const& str) {
    std::cerr << str << "\n";
}