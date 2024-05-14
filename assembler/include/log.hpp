#pragma once

#define LOG_PARSER
//#define LOG_INSTRUCTIONS
#define LOG_SYMBOL_TABLE

#include <string>

class Log {
public:
    static void STRING(const std::string &str);

    static void STRING_LN(std::string const &str);
};