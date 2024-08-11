#pragma once

#include "../../common/include/enum.h"

#include <list>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

static constexpr auto JMP_OVER_LITERALS = 4;
static constexpr auto DISPLACEMENT_SIZE_BYTES = 2;

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

    [[nodiscard]] static bool isIndexSymbolDefined(IndexSymbol &);

public:
    std::string linkerPath = "../../linker/bin/";
    std::string output = "obj.o";
    std::string outputTxt = "log.txt";
    std::string input;
    bool parsingJmp = false;

    std::vector<std::unique_ptr<Symbol>> symbols;
    std::vector<std::unique_ptr<Section>> sections;
    std::vector<std::unique_ptr<Relocation>> relocations;

    std::unordered_map<Symbol *, EquOperand *> equExpr;
    std::unordered_map<Symbol *, std::list<Instruction *>> equBackPatch;
    // tryToResolve for .word
    std::unordered_map<Symbol *, std::list<void *>> wordBackPatch;

    int32_t currSection = 0;

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

    void parseCondJmp(unsigned char, unsigned char, unsigned char, Operand *);        // done

    void parsePush(unsigned char);                      // done

    void parsePop(unsigned char);                       // done

    void parseNot(unsigned char);                       // done

    void parseXchg(unsigned char, unsigned char);       // done

    void parseTwoReg(unsigned char, unsigned char, unsigned char);  // done

    void parseCsrrd(unsigned char, unsigned char);      // done

    void parseCsrwr(unsigned char, unsigned char);      // done

    void parseLoad(Operand *, unsigned char);           // done

    void parseLoad(unsigned char, int16_t, unsigned char);           // done

    void parseStore(unsigned char, Operand *);          // done

    void parseStore(unsigned char, unsigned char, int16_t);          // done

    void parseRet();                                    // done

    void parseIRet();                                   // done

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

    IndexSymbol findSymbol(const std::string &) const;

    IndexSymbol declareSymbol(const std::string &);

    int32_t getSymbolIndex(const std::string &) const;

    Relocation *addRelLiteral(IndexSymbol, int = 0);

    Relocation *addRelSymbol(IndexSymbol, int = 0);

    Relocation *symbolAlreadyRelocated(IndexSymbol) const;

    Relocation *addRelLiteral(uint32_t, int = 0);

    void insertInstr(Instruction *instr);

    void correctRelocations();

    void deleteRelocations();

    void appendLiterals();

    IndexSymbol addSymbol(const std::string &, uint32_t, SCOPE, uint32_t, enum SYMBOL, SOURCE, enum DEFINED);

    [[nodiscard]] EquResolved tryToResolveEqu(EquOperand *);

    [[nodiscard]] IndexSymbol isSymbolDefined(const std::string &) const;

    [[nodiscard]] static bool isSymbolDeclared(IndexSymbol &);

    [[nodiscard]] bool isSymbolDeclared(const std::string &) const;

    [[nodiscard]] static bool isSymbolDefined(IndexSymbol &);

    [[nodiscard]] static bool isSymbolGlobal(IndexSymbol &);

    static void symbolNotDefined(const std::string &);

    [[nodiscard]]  bool isSymbolGlobal(const std::string &) const;

    RELOCATION getRelocationType(Symbol *, bool = false) const;

    uint32_t getValueOfSymbol(Symbol *) const;

};
