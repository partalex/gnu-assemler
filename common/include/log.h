#pragma once

class Log {
public:
    static const uint8_t LOG_FOOTER = 95;
    static const uint8_t LOG_TABLE_START = 5;
    static const char LOG_CHARACTER = '_';

    static void tableFooter(std::ostream &out) {
//    out << std::left << std::string(LOG_FOOTER, LOG_CHARACTER) << "\n" << "\n";
        out << "\n";
    }

    static void tableName(std::ostream &out, const std::string &str) {
        out << std::left << std::string(LOG_TABLE_START, LOG_CHARACTER) << str
            << std::string(LOG_FOOTER - LOG_TABLE_START - str.length(), LOG_CHARACTER) << "\n";
    }

};