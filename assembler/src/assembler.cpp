#include "../include/assembler.h"
#include "../include/lexer.h"
#include "../../common/include/operand.h"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iomanip>

extern int yylex_destroy(void);

extern int yyparse(void);

#define LOG_PARSER

std::unique_ptr<Assembler> Assembler::_instance = nullptr;

Assembler &Assembler::singleton() {
    if (!_instance)
        _instance = std::make_unique<Assembler>();
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
                    std::make_unique<Symbol>(temp->_symbol, false, UND, SCOPE::GLOBAL, 0, SYMBOL::SYMBOL));
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
    _sections[_currSectIndex]->addToLocCounter(literal);
}

void Assembler::parseEnd() {
#ifdef LOG_PARSER
    std::cout << "END" << "\n";
#endif
    writeTxt();
    if (hasUnresolvedSymbols()) {
        std::cerr << "Error: Unresolved symbols detected." << "\n";
        exit(EXIT_FAILURE);
    }
    writeElf64();
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
        symbol->_offset = _sections[_currSectIndex]->getLocCounter();
        symbol->_sectionIndex = _currSectIndex;
        symbol->_defined = true;
        symbol->_symbolType = SYMBOL::LABEL;
    } else {
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                true,
                _currSectIndex,
                SCOPE::LOCAL,
                _sections[_currSectIndex]->getLocCounter(),
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
        _sections[_currSectIndex]->write(temp->getValue(), _sections[_currSectIndex]->getLocCounter(), 4);
        _sections[_currSectIndex]->addToLocCounter(4);
        temp = temp->_next;
    }
    delete operand;
}

void Assembler::parseAscii(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "ASCII: " << str << "\n";
#endif
    _sections[_currSectIndex]->write((void *) str.c_str(), _sections[_currSectIndex]->getLocCounter(), str.length());
    _sections[_currSectIndex]->addToLocCounter(str.length());
}

void Assembler::parseHalt() {
#ifdef LOG_PARSER
    std::cout << "HALT" << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Halt_Instr>());
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef LOG_PARSER
    auto name = static_cast<enum INSTRUCTION>(inst);
    std::cout << name << "\n";
#endif
    _instructions.emplace_back(std::make_unique<NoAdr_Instr>(static_cast<enum INSTRUCTION>(inst)));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
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
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
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
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
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
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "PUSH: %r" << static_cast<int>(gpr) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Push_Instr>(gpr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "POP: %r" << static_cast<int>(gpr) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Pop_Instr>(gpr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "NOT" << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Not_Instr>(gpr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseInt(Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    // TODO
    _instructions.emplace_back(std::make_unique<Int_Instr>(std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << "XCHG: %r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Xchg_Instr>(regS, regD));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << static_cast<enum INSTRUCTION>(inst) << ": ";
    std::cout << "%r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    _instructions.emplace_back(std::make_unique<TwoReg_Instr>(static_cast<enum INSTRUCTION>(inst), regS, regD));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %" << Csr::CSR[csr] << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Csrrd_Instr>(csr, gpr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << static_cast<int>(gpr) << ", %" << Csr::CSR[csr] << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Csrwr_Instr>(gpr, csr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    std::cout << "%r, " << static_cast<int>(gpr);
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseLoad(unsigned char gpr1, unsigned char gpr2, int16_t offset) {
#ifdef LOG_PARSER
    std::cout << "LOAD: [%r" << static_cast<int>(gpr1) << "+" << offset << "], %r" << static_cast<int>(gpr2) << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Load_Instr>(gpr1, gpr2, offset));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr) << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    _instructions.emplace_back(std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand)));
    _sections[_currSectIndex]->writeInstr((void *) &_instructions.back()->_bytes,
                                          _sections[_currSectIndex]->getLocCounter());
    _sections[_currSectIndex]->addToLocCounter(4);
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
                    std::make_unique<Symbol>(temp->_symbol, false, INVALID, SCOPE::GLOBAL, 0, SYMBOL::SYMBOL));
        else {
            std::cerr << "Error: Symbol " << temp->_symbol << " already defined." << "\n";
            exit(EXIT_FAILURE);
        }
        temp = temp->_next;
    }
    delete list;
}

void Assembler::log(std::ostream &out) const {
    logSymbols(out);
    logSections(out);
    logInstructions(out);
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
        if (!sym->_defined && sym->_sectionIndex == INVALID)
            return true;
    return false;
}

void Assembler::logTableName(std::ostream &out, const std::string &str) {
    out << std::left << std::string(LOG_TABLE_START, LOG_CHARACTER) << str
        << std::string(LOG_FOOTER - LOG_TABLE_START - str.length(), LOG_CHARACTER) << "\n";
}

void Assembler::logTableFooter(std::ostream &out) {
//    out << std::left << std::string(LOG_FOOTER, LOG_CHARACTER) << "\n" << "\n";
    out << "\n";
}

void Assembler::logSections(std::ostream &out) const {
    logTableName(out, "Sections");
    Section::tableHeader(out);
    for (auto &sect: _sections)
        out << *sect;
    logTableFooter(out);
}

void Assembler::logSymbols(std::ostream &out) const {
    logTableName(out, "Symbols");
    Symbol::tableHeader(out);
    for (auto &sym: _symbols)
        out << *sym;
    logTableFooter(out);
}

void Assembler::logInstructions(std::ostream &out) const {
    logTableName(out, "Instructions");
    Instruction::tableHeader(out);
    for (auto &instr: _instructions)
        out << *instr;
    logTableFooter(out);
}

void Assembler::writeTxt() {
    std::ofstream out(_outputTxt);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open file " << _outputTxt << "\n";
        exit(EXIT_FAILURE);
    }
    log(out);
    for (auto &sect: _sections)
        sect->serialize(out);
//    for (auto &instr: _instructions)
//        out << instr->serialize();
    out.close();
}

void Assembler::fillElf64AMDHeader(Elf64_Ehdr &ehdr) {
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI] = ELFOSABI_SYSV;
    ehdr.e_ident[EI_ABIVERSION] = 0;
    ehdr.e_ident[EI_PAD] = 0;
    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_RISCV;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0;
    ehdr.e_phoff = 0;
    ehdr.e_shoff = 0;
    ehdr.e_flags = 0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = 0;
    ehdr.e_shnum = 0;
    ehdr.e_shstrndx = 0;
}

void Assembler::writeElf64() {
    std::ofstream out(_output, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open file " << _output << "\n";
        exit(EXIT_FAILURE);
    }

    Elf64_Ehdr ehdr;
    fillElf64AMDHeader(ehdr);

    // Za svaku sekciju u projektu
    for (auto &section: _sections) {
        // Kreiranje ELF sekcije
        Elf64_Shdr shdr;
        // Popunjavanje ELF sekcije sa odgovarajućim informacijama
        // Svi moji simboli se nalaze unutar _symbols
    }

    // Za svaki simbol u projektu
    for (auto &symbol: _symbols) {
        // Kreiranje ELF simbola
        Elf64_Sym sym;
        // Popunjavanje ELF simbola sa odgovarajućim informacijama
        // ...

    }

    // Upisivanje ELF zaglavlja, sekcija i simbola u izlazni fajl
    // ...

    out.write((char *) &ehdr, sizeof(Elf64_Ehdr));
    out.close();
}
