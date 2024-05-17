#include "loader_script_file.h"

#include <regex>
#include <string>
#include <iostream>
#include <algorithm>

std::regex section("^.(text|data|bss)(.[a-zA-Z_][a-zA-Z0-9]*)?$");
std::string allowedChars = "[_0-9a-zA-Z\\(\\)\\.,]";
std::regex assignment("^(" + allowedChars + "+)=(-?" + allowedChars + "+)((?:[\\+-]" + allowedChars + "+)*)$");

extern std::ofstream logFile;

void LoaderScriptFile::LoadFromFile(std::istream &inFile) {
    std::string line;

    while (getline(inFile, line)) {
        //strip whitespace
        line.erase(remove_if(line.begin(), line.end(), [](char x) { return std::isspace(x); }), line.end());

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
GetLabelVal(std::string label, unordered_map <std::string, Symbol> &symbols,
            std::unordered_map<std::string, int> &sectionPositions) {
    u_int32_t ret = 0;

    auto symbol = symbols.find(label);
    if (symbol == symbols.end()) {
        throw std::runtime_error("Symbol not found " + label);
    }

    if (symbol->second.symbolType == TokenType::LABEL || symbol->second.symbolType == TokenType::OPERAND_DEC) {
        ret += symbol->second.offset;
    }

    if (symbol->second.symbolType == TokenType::LABEL || symbol->second.symbolType == TokenType::SECTION) {
        std::string sectionName = symbol->second.sectionName;
        auto sectionPos = sectionPositions.find(sectionName);
        if (sectionPos == sectionPositions.end()) {
            throw std::runtime_error(
                    "Not allowed, recursive declaration (section position of label not yet calculated) " + sectionName);
        }

        ret += sectionPos->second;
    }

    return ret;
}

u_int32_t ParseOperand(std::string token, int dotVal, unordered_map <std::string, Symbol> &symbols,
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
    } else if (regex_match(token, base_match, tokenParsers[DOT])) {
        ret = dotVal;
    } else if (regex_match(token, base_match, tokenParsers[SCRIPTLABEL])) {
        //cout << base_match[1] << endl;
        //cout << base_match[0] << endl;
        ret = GetLabelVal(base_match[1], symbols, sectionPositions);
    } else if (regex_match(token, base_match, tokenParsers[ALIGN])) {

        u_int32_t first = ParseOperand(base_match[1], dotVal, symbols, sectionPositions);
        u_int32_t second = ParseOperand(base_match[2], dotVal, symbols, sectionPositions);

        if (first != first / 4 * 4) {
            first = (first / 4 + 1) * 4;
        }

        ret = first;
    }

    return ret;
}

void AddSymbol(std::unordered_map <std::string, Symbol> &symbols, Symbol &sym) {
    auto symbolOld = symbols.find(sym.name);

    if (symbolOld == symbols.end()) {
        symbols.insert({sym.name, sym});
    } else if (sym.defined) {
        if (symbolOld->second.defined) {
            throw runtime_error("Defined symbol twice ! " + symbolOld->second.name);
        } else {
            symbolOld->second.defined = true;
            symbolOld->second.offset = sym.offset;
            symbolOld->second.sectionName = sym.sectionName;
            symbolOld->second.scope = ScopeType::GLOBAL;
            symbolOld->second.symbolType = sym.symbolType;
        }
    }
}

void splitExpression(const std::string &s, std::vector <std::pair<std::string, bool>> &v) {
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

void LoaderScriptFile::FillSymbolsAndSectionPositions(std::unordered_map <std::string, Symbol> &symbols,
                                                      std::unordered_map<std::string, int> &sectionPositions) {
    u_int32_t locationCounter = 0;

//    /unordered_map<string, int> sectionPositions;

    for (auto &line: scriptInput) {
        //cout << line << endl;
        std::smatch base_match;
        if (regex_match(line, base_match, section)) {
            //TODO: check if section exists
            sectionPositions[symbols.find(line)->second.name] = locationCounter;
            locationCounter += symbols.find(line)->second.size;
        } else if (regex_match(line, base_match, assignment)) {
            //TODO: what have youuuu doneeeeeeeee
//            cout << "Whaaa" << endl;
//            cout << "left val " << base_match[1] << endl;
//            cout << "right val first " << base_match[2] << endl;
//            cout << "right val rest " << base_match[3] << endl;
            u_int32_t expressionVal = 0;

            ScriptOperandType tokenType1 = GetType(base_match[1]);

            if (tokenType1 != DOT && tokenType1 != SCRIPTLABEL) {
                throw std::runtime_error("Bad left operand ! " + base_match[1].str());
            }

            std::string firstRightOperand = base_match[2];

            bool negative = false;

            if (firstRightOperand[0] == '-') {
                firstRightOperand = firstRightOperand.substr(1);
                negative = true;
            }

            int firstRightOperandVal = ParseOperand(firstRightOperand, locationCounter, symbols, sectionPositions);

            expressionVal += (negative ? -1 : 1) * firstRightOperandVal;

            if (!base_match[3].str().empty()) {
                std::vector <std::pair<std::string, bool>> expressionTokens;
                splitExpression(base_match[3].str(), expressionTokens);

                for (auto &expr: expressionTokens) {
                    expressionVal += (expr.second ? -1 : 1) *
                                     ParseOperand(expr.first, locationCounter, symbols, sectionPositions);
                }

            }

            if (tokenType1 == DOT) {
                if (locationCounter > expressionVal)
                    throw std::runtime_error("Cannot assing smaller value to current address");
                locationCounter = expressionVal;
            } else if (tokenType1 == SCRIPTLABEL) {
                Symbol symbol(base_match[1].str(), true, "script", ScopeType::GLOBAL, expressionVal,
                              TokenType::OPERAND_DEC, 0);
                AddSymbol(symbols, symbol);
            }

        }
    }

}

