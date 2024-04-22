#include <iostream>
#include <fstream>
#include <cstring>
#include "assembler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "log.hpp"

extern int yylex_destroy(void);

extern int yyparse(void);

std::shared_ptr<Assembler> Assembler::_instance = nullptr;

Assembler &Assembler::singleton() {
    if (!_instance)
        _instance = std::make_shared<Assembler>();
    return *_instance;
}

int Assembler::pass(int argc, char **argv) {
    if (argc < 2)
        return EXIT_FAILURE;
    Assembler &as = Assembler::singleton();
    as._input = argv[argc - 1];
    for (int i = 1; i < argc - 1; ++i)
        if (!strcmp(argv[i], "-o")) {
            as._output = argv[++i];
            break;
        }
    freopen(as._input.c_str(), "r", stdin);
    if (yyparse())
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void Assembler::parseExtern(SymbolList *list) {
#ifdef DO_DEBUG
    Log::STRING("EXTERN: ");
    list->log();
#endif
    SymbolList *temp = list;
    while (temp) {
        auto entry = new SymbolTableEntry(
                EXTERN,
                temp->_symbol,
                nullptr,
                0,
                true
        );
        _table.addSymbol(temp->_symbol, entry);
        temp = temp->_next;
    }
}

void Assembler::parseSkip(int literal) {
#ifdef DO_DEBUG
    Log::STRING_LN("SKIP: " + std::to_string(literal));
#endif
}

void Assembler::parseEnd() {
#ifdef DO_DEBUG
    Log::STRING_LN("END");
#endif
    if (_table.hasUnresolvedSymbols()) {
        std::cerr << "Error: Unresolved symbols detected." << std::endl;
        exit(EXIT_FAILURE);
    }
    writeToFile();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("LABEL: " + str);
#endif
    auto entry = new SymbolTableEntry(
            LABEL,
            str,
            nullptr,
            _locationCounter
    );
    _table.addSymbol(str, entry);
}

void Assembler::parseSection(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("SECTION: " + str);
#endif
    SymbolTableEntry *entry = _table.getSymbol(str);
    if (entry == nullptr) {
        entry = new SymbolTableEntry(
                SECTION,
                str,
                nullptr,
                _locationCounter
        );
        _table.addSymbol(str, entry);
    }
    _currentSection = entry;
}

void Assembler::parseWord(Operand *operand) {
#ifdef DO_DEBUG
    Log::STRING("WORD: ");
    operand->log();
    Log::STRING_LN("");
#endif
}

void Assembler::parseAscii(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("ASCII: " + str);
#endif
    auto entry = new SymbolTableEntry(
            ASCII,
            str,
            _currentSection,
            _locationCounter
    );
    _table.addSymbol(str, entry);
    _locationCounter += str.length();
}

void Assembler::parseHalt() {
#ifdef DO_DEBUG
    Log::STRING_LN("HALT");
#endif

}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef DO_DEBUG
    auto name = static_cast<I::INSTRUCTION>(inst);
    Log::STRING_LN(I::NAMES[name]);
#endif
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef DO_DEBUG
    auto _inst = static_cast<I::INSTRUCTION>(inst);
    std::string name = I::NAMES[_inst];
    Log::STRING(name + ": ");
    operand->log();
    Log::STRING_LN("");
#endif
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("PUSH: %r" + std::to_string(gpr));
#endif
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("POP: %r" + std::to_string(gpr));
#endif
}

void Assembler::parseNot(unsigned char) {
#ifdef DO_DEBUG
    Log::STRING_LN("NOT");
#endif
}

void Assembler::parseInt(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("INT");
#endif
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef DO_DEBUG
    Log::STRING_LN("XCHG: " + std::to_string(regS) + ", " + std::to_string(regD));
#endif
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef DO_DEBUG
    auto name = static_cast<I::INSTRUCTION>(inst);
    Log::STRING(I::NAMES[name] + ": ");
    Log::STRING("%r" + std::to_string(regS) + ", ");
    Log::STRING_LN("%r" + std::to_string(regD));
#endif
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: %" + Csr::CSR[csr] + ", %r" + std::to_string(gpr));
#endif
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: %r" + std::to_string(gpr) + ", %" + Csr::CSR[csr]);
#endif
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING("LOAD: ");
    operand->logOne();
    Log::STRING_LN(", %r" + std::to_string(gpr));
#endif
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef DO_DEBUG
    Log::STRING("STORE: ");
    Log::STRING("%r" + std::to_string(gpr) + ", ");
    operand->logOne();
    Log::STRING_LN("");
#endif
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef DO_DEBUG
    Log::STRING("GLOBAL: ");
    list->log();
#endif
    SymbolList *temp = list;
    while (temp) {
        auto entry = new SymbolTableEntry(
                GLOBAL,
                temp->_symbol,
                nullptr,
                0,
                true
        );
        _table.addSymbol(temp->_symbol, entry);
        temp = temp->_next;
    }
}

void Assembler::writeToFile() {

}
