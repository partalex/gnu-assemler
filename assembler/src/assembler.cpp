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

void Assembler::parseNoAdr() {
#ifdef DO_DEBUG
    Log::STRING_LN("NOADR");
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

void Assembler::parseXchg(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("XCHG");
#endif
}

void Assembler::parseTwoReg(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("TWO REG");
#endif
}

void Assembler::parseCsrrd(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD");
#endif
}

void Assembler::parseCsrwr(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD");
#endif
}

void Assembler::parseLoad(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("LOAD");
#endif
}

void Assembler::parseStore(Operand *) {
#ifdef DO_DEBUG
    Log::STRING_LN("STORE");
#endif
}
