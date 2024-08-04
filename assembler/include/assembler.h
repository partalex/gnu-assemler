#pragma once

#include "../../common/include/enum.h"

#include <list>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class Symbol;

class Operand;

class Section;

class EquOperand;

class Relocation;

class SymbolList;

class Instruction;

class WordOperand;

class Assembler {
    static std::unique_ptr<Assembler> _instance;
public:
    std::string _linkerPath = "../../linker/bin/";
    std::string _output = "obj.o";
    std::string _outputTxt = "log.txt";
    std::string _input;

    std::vector<std::unique_ptr<Symbol>> _symbols;
    std::vector<std::unique_ptr<Section>> _sections;
    std::vector<std::unique_ptr<Relocation>> _relocations;

    std::unordered_map<Symbol *, std::unique_ptr<EquOperand>> _equExpr;
    std::unordered_map<Symbol *, std::list<Instruction *>> _equBackPatch;
    // tryToResolve for .word
    std::unordered_map<Symbol *, std::list<void *>> _wordBackPatch;

    int32_t _currSection = 0;

    ~Assembler() = default;

    void operator=(Assembler const &) = delete;

    static Assembler &singleton();                      // done

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

    void parseInt();                                    // done

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

    void parseRet();                                    // done

    void parseIRet();                                   // done

    std::pair<int32_t, Symbol *> findSymbol(const std::string &);

    void resolveEqu();

    void resolveWord();

    void checkUnresolvedSymbols();

    void logSections(std::ostream &) const;

    void logSymbols(std::ostream &) const;

    void logRelocations(std::ostream &out) const;

    static void symbolDuplicate(const std::string &);

    void writeTxt();

    void writeObj();

    void setOutput(char *string);

    void declareSymbol(const std::string &);

    void addRelIdent(const std::string &);

    void addRelLiteral(int32_t);

    void insertInstr(Instruction *instr);

    void correctRelocations();

};
