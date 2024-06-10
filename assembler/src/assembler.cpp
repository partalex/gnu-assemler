#include "../include/assembler.h"
#include "../include/lexer.h"
#include "../../common/include/operand.h"

#include <iostream>
#include <cstring>
#include <algorithm>

extern int yylex_destroy(void);

extern int yyparse(void);

#define LOG_PARSER

std::unique_ptr<Assembler> Assembler::_instance = nullptr;

Assembler &Assembler::singleton() {
    if (!_instance) {
        _instance = std::make_unique<Assembler>();
        _instance->_sections.emplace_back(std::make_unique<Section>("UND"));
    }
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
    list->log(std::cout);
#endif
    SymbolList *temp = list;
    while (temp) {
        auto it = findSymbol(temp->_symbol);
        if (it == nullptr)
            _symbols.emplace_back(
                    std::make_unique<Symbol>(temp->_symbol, false, _currSectIndex, SCOPE::GLOBAL, 0, SYMBOL::SYMBOL));
        else if (it->_scope == SCOPE::GLOBAL) {
            std::cerr << "Error: Symbol " << temp->_symbol << " already defined as global." << "\n";
            exit(EXIT_FAILURE);
        }
        temp = temp->_next;
    }
    delete list;
}

void Assembler::parseSkip(int literal) {
#ifdef LOG_PARSER
    std::cout << "SKIP: " << literal << "\n";
#endif
    _sections[_currSectIndex]->_size += literal;
}

void Assembler::parseEnd() {
#ifdef LOG_PARSER
    std::cout << "END" << "\n";
#endif
    log();
    if (hasUnresolvedSymbols()) {
        std::cerr << "Error: Unresolved symbols detected." << "\n";
        exit(EXIT_FAILURE);
    }
#ifdef LOG_SYMBOL_TABLE
    _symbolTable.log(std::cout);
#endif
#ifdef LOG_INSTRUCTIONS
    _instructions.log(std::cout);
#endif
}

void Assembler::parseLabel(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "LABEL: " << str << "\n";
#endif
    auto symbol = findSymbol(str);
    if (symbol != nullptr) {
        if (symbol->_defined) {
            std::cerr << "Error: Symbol " << str << " already defined." << "\n";
            exit(EXIT_FAILURE);
        }
        symbol->_offset = _sections[_currSectIndex]->_locationCounter;
        symbol->_sectionIndex = _currSectIndex;
        symbol->_defined = true;
        symbol->_symbolType = SYMBOL::LABEL;
    } else {
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                true,
                _currSectIndex,
                SCOPE::LOCAL,
                _sections[_currSectIndex]->_locationCounter,
                SYMBOL::LABEL
        ));
    }
}

void Assembler::parseSection(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "SECTION: " << str << "\n";
#endif
    auto it = std::find_if(_sections.begin(), _sections.end(), [&](const auto &section) {
        return section->_name == str;
    });
    if (it == _sections.end()) {
        _sections.emplace_back(std::make_unique<Section>(str));
        it = std::prev(_sections.end());
    }
    _currSectIndex = it - _sections.begin();
}

void Assembler::parseWord(WordOperand *operand) {
#ifdef LOG_PARSER
    std::cout << "WORD: ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    WordOperand *temp = operand;
    while (temp) {
        _sections[_currSectIndex]->write(temp->getValue(), _sections[_currSectIndex]->_locationCounter, 4);
        _sections[_currSectIndex]->_locationCounter += 4;
        temp = temp->_next;
    }
    delete operand;
}

void Assembler::parseAscii(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "ASCII: " << str << "\n";
#endif
    _sections[_currSectIndex]->write((void *) str.c_str(), str.length(), _sections[_currSectIndex]->_locationCounter);
    _sections[_currSectIndex]->_locationCounter += str.length();
}

void Assembler::parseHalt() {
#ifdef LOG_PARSER
    std::cout << "HALT" << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Halt_Instr>());
}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef LOG_PARSER
    auto name = static_cast<enum INSTRUCTION>(inst);
    std::cout << name << "\n";
#endif
    _instructions.emplace_back(std::make_unique<NoAdr_Instr>(static_cast<enum INSTRUCTION>(inst)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Jmp_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                           std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseCall(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Call_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                            std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseCondJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<JmpCond_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                               std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "PUSH: %r" << static_cast<int>(gpr) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Push_Instr>(gpr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "POP: %r" << static_cast<int>(gpr) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Pop_Instr>(gpr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "NOT" << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Not_Instr>(gpr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseInt(Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    // TODO
    _instructions.emplace_back(std::make_unique<Int_Instr>(std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << "XCHG: %r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Xchg_Instr>(regS, regD));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << static_cast<enum INSTRUCTION>(inst) << ": ";
    std::cout << "%r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    _instructions.emplace_back(std::make_unique<TwoReg_Instr>(static_cast<enum INSTRUCTION>(inst), regS, regD));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %" << Csr::CSR[csr] << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    _instructions.emplace_back(std::make_unique<Csrrd_Instr>(csr, gpr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << static_cast<int>(gpr) << ", %" << Csr::CSR[csr] << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Csrwr_Instr>(gpr, csr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    operand->log(std::cout);
    std::cout << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr);
    _instructions.emplace_back(std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr) << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->_locationCounter += 4;
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef LOG_PARSER
    std::cout << "GLOBAL: ";
    list->log(std::cout);
#endif
    SymbolList *temp = list;
    while (temp) {
        auto it = findSymbol(temp->_symbol);
        if (it == nullptr)
            _symbols.emplace_back(
                    std::make_unique<Symbol>(temp->_symbol, false, _currSectIndex, SCOPE::GLOBAL, 0, SYMBOL::SYMBOL));
        else {
            std::cerr << "Error: Symbol " << temp->_symbol << " already defined." << "\n";
            exit(EXIT_FAILURE);
        }
        temp = temp->_next;
    }
    delete list;
}

void Assembler::log() const {
    logSymbols();
    logSections();
    logInstructions();
}

Symbol *Assembler::findSymbol(std::string symName) {
    auto it = std::find_if(_symbols.begin(), _symbols.end(), [&](const auto &symbol) {
        return symbol->_name == symName;
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

void Assembler::logSections() const {
    std::cout << "----------------------------------------\n";
    Section::tableHeader(std::cout);
    for (auto &sect: _sections)
        std::cout << *sect;
    std::cout << "----------------------------------------\n";
}

void Assembler::logSymbols() const {
    std::cout
            << "----------------------------------------------------------------------------------------------------\n";
    Symbol::tableHeader(std::cout);
    for (auto &sym: _symbols)
        std::cout << *sym;
    std::cout
            << "----------------------------------------------------------------------------------------------------\n";
}

void Assembler::logInstructions() const {
    std::cout << "----------------------------------------------------\n";
    Instruction::tableHeader(std::cout);
    for (auto &instr: _instructions)
        std::cout << *instr;
    std::cout << "----------------------------------------------------\n";
}
