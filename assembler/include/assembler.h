#pragma once

#include "../../common/include/log.h"
#include "../../common/include/symbol.h"
#include "../../common/include/section.h"
#include "../../common/include/structures.h"
#include "../../common/include/relocation.h"
#include "../../common/include/instruction.h"

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>

class WordOperand;

class Assembler {

    uint64_t _txtSectIndex = UNDEFINED;

    static std::unique_ptr<Assembler> _instance;
    std::string _output = "obj.o";
    std::string _outputTxt = "log.txt";
    std::string _input;

    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::vector<std::unique_ptr<Section>> _sections;
    std::vector<std::unique_ptr<Relocation>> _relocations;

    std::unordered_map<Symbol *, std::unique_ptr<EquOperand>> _equExpr;
    std::unordered_map<Symbol *, std::list<Instruction *>> _equBackPatch;

    int32_t _currSectIndex = 0;

public:

    ~Assembler() = default;

    void operator=(Assembler const &) = delete;

    static Assembler &singleton();

    static int pass(int, char **);                      // done

    void log(std::ostream &) const;                     // done

    void parseLabel(const std::string &);               // done

    void parseEnd();                                    // done

    void parseGlobal(SymbolList *);                     // done

    void parseExtern(SymbolList *);                     // done

    void parseSection(const std::string &);             // done

    void parseWord(WordOperand *);                      // done

    void parseEqu(const std::string &, EquOperand *);   // done

    void parseAscii(const std::string &);               // done

    void parseSkip(int);                                // done

    // instructions
    void parseHalt();                                   // done

    void parseInt(Operand *);                           // done

    void parseJmp(unsigned char, Operand *);            // done

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

    void parseLoad(unsigned char, unsigned char, int16_t);           // done

    void parseStore(unsigned char, Operand *);          // done

    void parseNoAdr(unsigned char);                     // done

    std::pair<int32_t, Symbol *> findSymbol(const std::string &);

    void resolveEqu();

    bool hasUnresolvedSymbols();

    void logSections(std::ostream &) const;

    void logSymbols(std::ostream &) const;

    void logRelocations(std::ostream &out) const;

    static void symbolDuplicate(const std::string &);

    void writeTxt();

    void writeObj();

    void setOutput(char *string);

    void insertInstr(Instruction *instr);

    void addRelToInstr(Operand *, RELOCATION = R_2B_EXC_4b, uint32_t = 2);

};
