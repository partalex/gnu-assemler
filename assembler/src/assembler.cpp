#include "../include/assembler.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../../common/include/operand.h"

#include <iostream>
#include <cstring>
#include <algorithm>

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
#ifdef LOG_PARSER
    std::cout << "EXTERN: ";
    list->log();
#endif
    SymbolList *temp = list;
    while (temp) {
        addNewSymbol(temp->_symbol, false, SYMBOL::SYMBOL, SECTION_TYPE::UND, SCOPE::GLOBAL, 0);
        temp = temp->_next;
    }
    delete list;
}

void Assembler::parseSkip(int literal) {
#ifdef LOG_PARSER
    std::cout<< "SKIP: " << literal << "\n";;
#endif
    // initialise memory with 0
    _locationCounter += literal;
}

void Assembler::parseEnd() {
#ifdef LOG_PARSER
    std::cout<< "END" << "\n";;
#endif
    if (hasUnresolvedSymbols()) {
        std::cerr << "Error: Unresolved symbols detected." << "\n";;
        exit(EXIT_FAILURE);
    }
#ifdef LOG_SYMBOL_TABLE
    _symbolTable.log();
#endif
#ifdef LOG_INSTRUCTIONS
    _instructions.log();
#endif
    writeToFile();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "LABEL: " << str << "\n";;
#endif
    auto symbol = findSymbol(str, SYMBOL::SYMBOL);
    if (symbol != nullptr) {
        if (symbol->_defined) {
            std::cerr << "Error: Symbol " << str << " already defined." << "\n";;
            exit(EXIT_FAILURE);
        }
        symbol->_offset = _locationCounter;
        symbol->_sectionIndex = _currentSection;
        symbol->_defined = true;
    } else {
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                true,
                _currentSection,
                SCOPE::LOCAL,
                _locationCounter,
                SYMBOL::SYMBOL
        ));
    }
}

void Assembler::parseSection(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "SECTION: " << str << "\n";;3
#endif
    auto entry = findSymbol(str, SYMBOL::SECTION);
    if (entry != nullptr) {
        _currentSection = entry->_sectionIndex;
        _locationCounter = entry->_offset;
    } else {
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                true,
                _currentSection,
                SCOPE::LOCAL,
                _locationCounter,
                SYMBOL::SECTION
        ));
    }
    _locationCounter = 0;
}

void Assembler::parseWord(WordOperand *operand) {
#ifdef LOG_PARSER
    std::cout << operand->getValue();
    operand->log();
    std::cout << "\n";
#endif
    // TODO
    WordOperand *temp = operand;
    while (temp) {
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
#ifdef LOG_PARSER
    std::cout << "ASCII: " << str << "\n";;
#endif
//    auto entry = std::make_unique<SymbolTableEntry>(
//            0,
//            str.length(),
//            EntryType::ASCII,
//            Bind::LOCAL,
//            _currentSection,
//            str
//    );
//    _symbolTable.addSymbol(std::move(entry));
    _locationCounter += str.length();
}

void Assembler::parseHalt() {
#ifdef LOG_PARSER
    std::cout << "HALT" << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Halt_Instr>());
}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef LOG_PARSER
    auto name = static_cast<I::INSTRUCTION>(inst);
    std::cout << I::NAMES[name] << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<NoAdr_Instr>(static_cast<enum INSTRUCTION>(inst)));
    _locationCounter += 4;
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<I::INSTRUCTION>(inst);
    std::cout << I::NAMES[_inst] << ": ";
    operand->log();
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Jmp_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                           std::unique_ptr<Operand>(operand)));
    _locationCounter += 4;
}

void Assembler::parseCall(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << I::NAMES[_inst] << ": ";
    operand->log();
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Call_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                            std::unique_ptr<Operand>(operand)));
    _locationCounter += 4;
}

void Assembler::parseCondJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << I::NAMES[_inst] << ": ";
    operand->log();
    std::cout << "\n";
#endif
    std::cout << "\n";

    _instructions.emplace_back(std::make_unique<JmpCond_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                               std::unique_ptr<Operand>(operand)));
    _locationCounter += 4;
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "PUSH: %r" << gpr << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Push_Instr>(gpr));
    _locationCounter += 4;
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef LOG_PARSER
    sdt::cout<< ""
    sdt::cout<< "POP: %r" <<  gpr << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Pop_Instr>(gpr));
    _locationCounter += 4;
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout<<"NOT" << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Not_Instr>(gpr));
    _locationCounter += 4;
}

void Assembler::parseInt(Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    // TODO
    _instructions.emplace_back(std::make_unique<Int_Instr>(std::unique_ptr<Operand>(operand)));
    _locationCounter += 4;
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << "XCHG: %r" << regS << ", %r" << regD << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Xchg_Instr>(regS, regD));
    _locationCounter += 4;
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << static_cast<enum INSTRUCTION>(inst) << ": ";
    std::cout << "%r" << regD << "\n";;
    std::cout << "%r" << regS << ", %r" << regD << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<TwoReg_Instr>(static_cast<enum INSTRUCTION>(inst), regS, regD));
    _locationCounter += 4;
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %" << Csr::CSR[csr] << ", %r" << gpr << "\n";;
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    _instructions.emplace_back(std::make_unique<Csrrd_Instr>(csr, gpr));
    _locationCounter += 4;
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << gpr << ", %" << Csr::CSR[csr] << "\n";;
#endif
    _instructions.emplace_back(std::make_unique<Csrwr_Instr>(gpr, csr));
    _locationCounter += 4;
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    operand->log();
    std::cout << ", %r" << gpr << "\n";;
#endif
    auto instr = std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr);
    _instructions.emplace_back(std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr));
    _locationCounter += 4;
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << gpr << ", ";
    operand->log();
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand)));
    _locationCounter += 4;
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef LOG_PARSER
    std::cout << "GLOBAL: ";
    list->log();
#endif
    SymbolList *temp = list;
    while (temp) {
        addNewSymbol(temp->_symbol, false, SYMBOL::SYMBOL, SECTION_TYPE::UND, SCOPE::GLOBAL, 0);
        temp = temp->_next;
    }
    delete list;
}

void Assembler::writeToFile() {
    std::cout << "Writing to file: " << _output << "\n";;
}

void Assembler::log() {
#ifdef LOG_SYMBOL_TABLE
    _symbolTable.log();
#endif
#ifdef LOG_INSTRUCTIONS
    _instructions.log();
#endif
}

void Assembler::addNewSymbol(std::string symName, bool symDefined, enum SYMBOL symbolType, SECTION_TYPE symSection,
                             SCOPE symScope, uint32_t locationCounter) {
    if (std::any_of(_symbols.begin(), _symbols.end(), [&](const auto &symbol) {
        return symbol->_name == symName && symbol->_symbolType == symbolType && symbol->_defined;
    })) {
        std::cerr << "Error: Symbol " << symName << " already defined." << "\n";
        exit(EXIT_FAILURE);
    }
    _symbols.emplace_back(
            std::make_unique<Symbol>(symName, symDefined, _currentSection, symScope, locationCounter, symbolType));
}

Symbol *Assembler::findSymbol(std::string symName, enum SYMBOL symType) {
    auto it = std::find_if(_symbols.begin(), _symbols.end(), [&](const auto &symbol) {
        return symbol->_name == symName && symbol->_symbolType == symType;
    });
    if (it == _symbols.end())
        return nullptr;
    return it->get();
}

bool Assembler::hasUnresolvedSymbols() {
    for (auto &sym: _symbols)
        if (!sym->_defined)
            return true;
    return false;
}
