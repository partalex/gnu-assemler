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
        auto entry = SymbolTableEntry(
                0,
                0,
                NO_TYPE,
                GLOBAL,
                0,
                temp->_symbol
        );
        _symbolTable.addSymbol(entry);
        temp = temp->_next;
    }
    delete list;
}

void Assembler::parseSkip(int literal) {
#ifdef DO_DEBUG
    Log::STRING_LN("SKIP: " + std::to_string(literal));
#endif
    _locationCounter += literal;
}

void Assembler::parseEnd() {
#ifdef DO_DEBUG
    Log::STRING_LN("END");
#endif
    if (_symbolTable.hasUnresolvedSymbols()) {
        std::cerr << "Error: Unresolved symbols detected." << std::endl;
        exit(EXIT_FAILURE);
    }
    _instructions.log();
    writeToFile();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("LABEL: " + str);
#endif
    auto entry = SymbolTableEntry(
            0,
            0,
            NO_TYPE,
            LOCAL,
            _currentSection,
            str
    );
    _symbolTable.addSymbol(entry);
}

void Assembler::parseSection(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("SECTION: " + str);
#endif
    SymbolTableEntry *entry = _symbolTable.getSymbol(LOCAL, SECTION, str);
    if (entry == nullptr) {
        auto temp = SymbolTableEntry(
                0,
                0,
                SECTION,
                LOCAL,
                ++_currentSection,
                str
        );
        entry = &temp;
        _symbolTable.addSymbol(*entry);
    }
    _currentSection = entry->_ndx;
    _locationCounter = 0;
}

void Assembler::parseWord(WordOperand *operand) {
#ifdef DO_DEBUG
    Log::STRING("WORD: ");
    operand->log();
    Log::STRING_LN("");
#endif
    // TODO
    Operand *temp = operand;
    while (temp) {
        // TODO
        // Alocirajte 4 bajta prostora za svaki Operand
        // Inicijalizujte alocirani prostor vrednostima Operand-a
        // Ovde pretpostavljamo da imate metodu koja može da vrati vrednost Operand-a
        // Ako ne, moraćete da dodate takvu metodu u svoje Operand klase
        // uint32_t value = temp->getValue();
        // Dodajte kod za alociranje i inicijalizaciju prostora ovde
        temp = temp->_next;
    }

    delete operand;
    _locationCounter += 4;
}

void Assembler::parseAscii(const std::string &str) {
#ifdef DO_DEBUG
    Log::STRING_LN("ASCII: " + str);
#endif
    auto entry = SymbolTableEntry(
            0,
            str.length(),
            ASCII,
            LOCAL,
            _currentSection,
            str
    );
    _symbolTable.addSymbol(entry);
    _locationCounter += str.length();
}

void Assembler::parseHalt() {
#ifdef DO_DEBUG
    Log::STRING_LN("HALT");
#endif
    auto instr = std::make_unique<Halt_Instr>();
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef DO_DEBUG
    auto name = static_cast<I::INSTRUCTION>(inst);
    Log::STRING_LN(I::NAMES[name]);
#endif
    auto instr = std::make_unique<NoAdr_Instr>(static_cast<I::INSTRUCTION>(inst));
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef DO_DEBUG
    auto _inst = static_cast<I::INSTRUCTION>(inst);
    std::string name = I::NAMES[_inst];
    Log::STRING(name + ": ");
    operand->log();
    Log::STRING_LN("");
#endif
    auto instr = std::make_unique<Jmp_Instr>(static_cast<I::INSTRUCTION>(inst), std::unique_ptr<Operand>(operand));
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("PUSH: %r" + std::to_string(gpr));
#endif
    auto instr = std::make_unique<Push_Instr>(gpr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("POP: %r" + std::to_string(gpr));
#endif
    auto instr = std::make_unique<Pop_Instr>(gpr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("NOT");
#endif
    auto instr = std::make_unique<Not_Instr>(gpr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseInt(Operand *operand) {
#ifdef DO_DEBUG
    Log::STRING_LN("INT");
#endif
    // TODO
    auto instr = std::make_unique<Int_Instr>(std::unique_ptr<Operand>(operand));
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef DO_DEBUG
    Log::STRING_LN("XCHG: " + std::to_string(regS) + ", " + std::to_string(regD));
#endif
    auto instr = std::make_unique<Xchg_Instr>(regS, regD);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef DO_DEBUG
    auto name = static_cast<I::INSTRUCTION>(inst);
    Log::STRING(I::NAMES[name] + ": ");
    Log::STRING("%r" + std::to_string(regS) + ", ");
    Log::STRING_LN("%r" + std::to_string(regD));
#endif
    auto instr = std::make_unique<TwoReg_Instr>(static_cast<I::INSTRUCTION>(inst), regS, regD);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: %" + Csr::CSR[csr] + ", %r" + std::to_string(gpr));
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef DO_DEBUG
    Log::STRING_LN("CSRRD: %r" + std::to_string(gpr) + ", %" + Csr::CSR[csr]);
#endif
    auto instr = std::make_unique<Csrwr_Instr>(gpr, csr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef DO_DEBUG
    Log::STRING("LOAD: ");
    operand->logOne();
    Log::STRING_LN(", %r" + std::to_string(gpr));
#endif
    auto instr = std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr);
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef DO_DEBUG
    Log::STRING("STORE: ");
    Log::STRING("%r" + std::to_string(gpr) + ", ");
    operand->logOne();
    Log::STRING_LN("");
#endif
    auto instr = std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand));
    _instructions.addInstruction(std::move(instr));
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef DO_DEBUG
    Log::STRING("GLOBAL: ");
    list->log();
#endif
    SymbolList *temp = list;
    while (temp) {
        auto entry = SymbolTableEntry(
                0,
                0,
                NO_TYPE,
                GLOBAL,
                0,
                temp->_symbol
        );
        _symbolTable.addSymbol(entry);
        temp = temp->_next;
    }
    delete list;
}

void Assembler::writeToFile() {

}

void Assembler::log() {
    _symbolTable.log();
    _instructions.log();
}
