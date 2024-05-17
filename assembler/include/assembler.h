#pragma once

#include "../../common/include/structures.h"

#include <memory>
#include <vector>
#include <unordered_map>

class WordOperand;

class Assembler {
    static std::shared_ptr<Assembler> _instance;
    std::string _output = "output/obj.o";
    std::string _input;

    SymbolTable _symbolTable;
    std::unordered_map<std::string, RelocationTable> _relTables;
    Instructions _instructions;

    uint32_t _locationCounter = 0;
    uint32_t _currentSection = 0;

public:
    ~Assembler() = default;

    void operator=(Assembler const &) = delete;

    static Assembler &singleton();

    static int pass(int, char **);                      // done

    void log();                                         // TODO

    void parseLabel(const std::string &);               // done

    void parseEnd();                                    // done

    void parseGlobal(SymbolList *);                     // done

    void parseExtern(SymbolList *);                     // done

    void parseSection(const std::string &);             // done

    void parseWord(WordOperand *);                      // TODO

    void parseAscii(const std::string &);               // done

    void parseSkip(int);                                // TODO

    // instructions
    void parseHalt();                                   // done

    void parseInt(Operand *);                           // TODO

    void parseJmp(unsigned char, Operand *);            // TODO

    void parseCall(unsigned char, Operand *);           // done

    void parseCondJmp(unsigned char, Operand *);        // done

    void parsePush(unsigned char);                      // done

    void parsePop(unsigned char);                       // done

    void parseNot(unsigned char);                       // done

    void parseXchg(unsigned char, unsigned char);       // done

    void parseTwoReg(unsigned char, unsigned char, unsigned char);  // done

    void parseCsrrd(unsigned char, unsigned char);      // done

    void parseCsrwr(unsigned char, unsigned char);      // done

    void parseLoad(Operand *, unsigned char);           // done

    void parseStore(unsigned char, Operand *);          // done

    void parseNoAdr(unsigned char);                     // TODO

    void writeToFile();
};
