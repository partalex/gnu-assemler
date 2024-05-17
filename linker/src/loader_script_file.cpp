#include "../include/loader_script_file.h"

#include <regex>
#include <string>
#include <iostream>
#include <algorithm>

std::regex section("^.(text|data|bss)(.[a-zA-Z_][a-zA-Z0-9]*)?$");
std::string allowedChars = "[_0-9a-zA-Z\\(\\)\\.,]";
std::regex assignment("^(" + allowedChars + "+)=(-?" + allowedChars + "+)((?:[\\+-]" + allowedChars + "+)*)$");

extern std::ofstream logFile;

void LoaderScriptFile::loadFromFile(std::istream &inFile) {
    std::string line;
    while (getline(inFile, line)) {
        line.erase(remove_if(line.begin(), line.end(), [](char x) {
            return std::isspace(x);
        }), line.end());
        if (!line.empty())
            scriptInput.push_back(line);
    }
}

enum ScriptOperandType {
    DOT, DEC, HEX, SCRIPTLABEL, ALIGN
};

std::unordered_map<int, std::regex> tokenParsers =
        {
                {DOT,         std::regex("^\\.$")},
                {DEC,         std::regex("^([0-9]+)$")},
                {HEX,         std::regex("^(0x[0-9abcdef]+)$")},
                {SCRIPTLABEL, std::regex("^([_a-zA-Z][a-zA-Z0-9]*)$")},
                {ALIGN,       std::regex("^align\\(([_a-zA-Z][a-zA-Z0-9]*|\\.),([0-9]+|0x[0-9abcdef]+)\\)$")},
        };

ScriptOperandType GetType(std::string token) {
    bool defined = false;
    ScriptOperandType ret;
    for (auto &parseRule: tokenParsers) {
        if (parseRule.first == SYMBOL)
            continue;
        if (regex_match(token, parseRule.second)) {
            if (!defined) {
                defined = true;
                ret = (ScriptOperandType) parseRule.first;
            } else
                throw std::runtime_error("Ambigous token! " + token);
        }
    }
    if (!defined)
        throw std::runtime_error("Illegal token! " + token);
    return ret;
}

u_int32_t
GetLabelVal(std::string label, std::unordered_map<std::string, Symbol> &symbols,
            std::unordered_map<std::string, int> &sectionPositions) {
    u_int32_t ret = 0;

    auto symbol = symbols.find(label);
    if (symbol == symbols.end()) {
        throw std::runtime_error("Symbol not found " + label);
    }

    if (symbol->second._symbolTypesymbolType == SYMBOL::LABEL || symbol->second._symbolType == SYMBOL::OPERAND_DEC)
        ret += symbol->second._offset;

    if (symbol->second._symbolType == SYMBOL::LABEL || symbol->second._symbolType == SYMBOL::SECTION) {
        std::string sectionName = symbol->second._sectionName;
        auto sectionPos = sectionPositions.find(sectionName);
        if (sectionPos == sectionPositions.end()) {
            throw std::runtime_error(
                    "Not allowed, recursive declaration (section position of label not yet calculated) " + sectionName);
            ret += sectionPos->second;
        }
        return ret;
    }

    u_int32_t ParseOperand(std::string token, int dotVal, std::unordered_map<std::string, Symbol> &symbols,
                           std::unordered_map<std::string, int> &sectionPositions) {
        std::smatch base_match;
        u_int32_t ret;

        if (regex_match(token, base_match, tokenParsers[DEC])) {
            std::stringstream ss;
            ss << base_match[1];
            ss >> ret;
        } else if (regex_match(token, base_match, tokenParsers[HEX])) {
            std::stringstream ss;
            ss << base_match[1];
            ss >> std::hex >> ret;
        } else if (regex_match(token, base_match, tokenParsers[DOT]))
            ret = dotVal;
        else if (regex_match(token, base_match, tokenParsers[SCRIPTLABEL]))
            ret = GetLabelVal(base_match[1], symbols, sectionPositions);
        else if (regex_match(token, base_match, tokenParsers[ALIGN])) {
            u_int32_t first = ParseOperand(base_match[1], dotVal, symbols, sectionPositions);
            if (first != first / 4 * 4)
                first = (first / 4 + 1) * 4;
            ret = first;
        }
        return ret;
    }

    void AddSymbol(std::unordered_map<std::string, Symbol> &symbols, Symbol &sym) {
        auto symbolOld = symbols.find(sym._name);

        if (symbolOld == symbols.end()) {
            symbols.insert({sym._name, sym});
        } else if (sym._defined) {
            if (symbolOld->second.defined) {
                throw std::runtime_error("Defined symbol twice ! " + symbolOld->second._name);
            } else {
                symbolOld->second._defined = true;
                symbolOld->second._offset = sym._offset;
                symbolOld->second._sectionName = sym._sectionName;
                symbolOld->second._scope = SCOPE::GLOBAL;
                symbolOld->second._symbolType = sym._symbolType;
            }
        }
    }

    void splitExpression(const std::string &s, std::vector<std::pair<std::string, bool>> &v) {
        // to avoid modifying original string
        // first duplicate the original string and return a char pointer then free the memory
        static std::regex r("^([\\+-])(" + allowedChars + "+).*");

        std::string processMe = s;
        std::smatch base_match;

        while (regex_match(processMe, base_match, r)) {
            //cout << "found " << base_match[0] << " " << base_match[1] << " " << base_match[2] << endl;
            v.push_back({base_match[2], base_match[1] == "-"});

            size_t slice = processMe.find_first_of("+-", 1);

            if (slice == std::string::npos)
                return;


            processMe = processMe.substr(slice);
        }

        throw std::runtime_error("Irregular expression !");
    }

    void LoaderScriptFile::fillSymbolsAndSectPos(std::unordered_map<std::string, Symbol> &,
                                                 std::unordered_map<std::string, int> &) {
        u_int32_t locationCounter = 0;

//    /unordered_map<string, int> sectionPositions;

        for (auto &line: scriptInput) {
            //cout << line << endl;
            std::smatch base_match;
            if (regex_match(line, base_match, section)) {
                sectionPositions[symbols.find(line)->second.name] = locationCounter;
                locationCounter += symbols.find(line)->second.size;
            } else if (regex_match(line, base_match, assignment)) {
                u_int32_t expressionVal = 0;
                ScriptOperandType SYMBOL1 = GetType(base_match[1]);
                if (SYMBOL1 != DOT && SYMBOL1 != SCRIPTLABEL)
                    throw std::runtime_error("Bad left operand ! " + base_match[1].str());
                std::string firstRightOperand = base_match[2];
                bool negative = false;
                if (firstRightOperand[0] == '-') {
                    firstRightOperand = firstRightOperand.substr(1);
                    negative = true;
                }
                int firstRightOperandVal = ParseOperand(firstRightOperand, locationCounter, symbols, sectionPositions);
                expressionVal += (negative ? -1 : 1) * firstRightOperandVal;
                if (!base_match[3].str().empty()) {
                    std::vector<std::pair<std::string, bool>> expressionTokens;
                    splitExpression(base_match[3].str(), expressionTokens);
                    for (auto &expr: expressionTokens)
                        expressionVal += (expr.second ? -1 : 1) *
                                         ParseOperand(expr.first, locationCounter, symbols, sectionPositions);
                }
                if (SYMBOL1 == DOT) {
                    if (locationCounter > expressionVal)
                        throw std::runtime_error("Cannot assing smaller value to current address");
                    locationCounter = expressionVal;
                } else if (SYMBOL1 == SCRIPTLABEL) {
                    Symbol symbol(base_match[1].str(), true, "script", SCOPE::GLOBAL, expressionVal,
                                  SYMBOL::OPERAND_DEC, 0);
                    AddSymbol(symbols, symbol);
                }
            }
        }

    }
}

