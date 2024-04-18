#include <iostream>
#include <fstream>
#include <cstring>
#include "assembler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "log.hpp"

//extern int yylex_destroy(void);

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
    Log::STRING_LN("EXTERN: ");
    list->log();
#endif
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
}

void Assembler::parseLabel(const std::string &str) {
    singleton()._labels.push_back(str);
#ifdef DO_DEBUG
    Log::STRING_LN("LABEL: " + str);
#endif
}

void Assembler::parseSection(const std::string &str) {
    singleton()._sections.push_back(str);
#ifdef DO_DEBUG
    Log::STRING_LN("SECTION: " + str);
#endif
}

void Assembler::parseWord(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("WORD");
#endif

}

void Assembler::parseAscii(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("ASCII: " + str);
#endif
}

void Assembler::parseHalt() {
#ifdef DO_DEBUG
    Log::STRING_LN("HALT");
#endif
}

void Assembler::parseNoAdr(unsigned char reg) {
#ifdef DO_DEBUG
    Log::STRING_LN("NOADR: " + std::to_string(reg));
#endif
}

void Assembler::parseJmp(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("JMP");
#endif

}

void Assembler::parsePush(unsigned char) {
#ifdef DO_DEBUG
    Log::STRING_LN("PUSH");
#endif
}

void Assembler::parsePop(unsigned char) {
#ifdef DO_DEBUG
    Log::STRING_LN("POP");
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

void Assembler::parseTwoReg(unsigned char oc, unsigned char regS, unsigned char regD) {
#ifdef DO_DEBUG
    Log::STRING_LN("TWO REG: " + std::to_string(oc) + ", " + std::to_string(regS) + ", " + std::to_string(regD));
#endif
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: " + std::to_string(csr) + ", " + std::to_string(gpr));
#endif
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: " + std::to_string(gpr) + ", " + std::to_string(csr));
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
}
