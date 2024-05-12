//#define DO_DEBUG

#pragma once

#include <string>

class Log {
public:
    static void STRING(const std::string &str);

    static void STRING_LN(std::string const &str);
};