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
            as.setOutput(argv[++i]);
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
        auto symbol = findSymbol(temp->_symbol);
        if (symbol.first == -1)
            _symbols.emplace_back(
                    std::make_unique<Symbol>(temp->_symbol, UNDEFINED, SCOPE::GLOBAL, 0, SYMBOL::LABEL));
        else if (symbol.second->core.flags.scope == SCOPE::GLOBAL) {
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
    writeObj();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "LABEL: " << str << "\n";
#endif
    auto symbol = findSymbol(str);
    if (symbol.first != -1) {
        // Symbol is in symbols

        if (symbol.second->core.sectionIndex < UNDEFINED)
            symbolDuplicate(symbol.second->core.name);
//        else if (symbol.second->core.flags.scope == LOCAL)
//            symbolDuplicate(symbol.second->core.name);

        // Set symbol
        symbol.second->core.offset = _sections[_currSectIndex]->getLocCounter();
        symbol.second->core.sectionIndex = _currSectIndex;
        symbol.second->core.flags.symbolType = SYMBOL::LABEL;
    } else {
        // Symbol does not exist, add it
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
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
        return section->core.name == str;
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
        if (temp->isLabel()) {
            // If operand is label

            auto label = temp->stringValue();
            auto symbol = findSymbol(label);

            if (symbol.first != -1)
                // Symbol is in symbols

                if (symbol.second->core.sectionIndex <= UNDEFINED) {
                    // If symbol is defined
                    Section *section = _sections[symbol.second->core.sectionIndex].get();
                    auto *ptr = section->core.data.data() + symbol.second->core.offset;
                    _sections[_currSectIndex]->write(ptr, _sections[_currSectIndex]->getLocCounter(), 4);

                } else {

                    // Symbol is not defined
                    _relocations.emplace_back(std::make_unique<Relocation>(
                            label,
                            symbol.first,
                            _currSectIndex,
                            _sections[_currSectIndex]->getLocCounter(),
                            4,
                            RELOCATION::R_2B_EXC_4b
                    ));
                    _sections[_currSectIndex]->addToLocCounter(4);
                }

            else {
                // Symbol does not exist, exit
                std::cerr << "Error: Symbol inside .word " << label << " not defined." << "\n";
                exit(EXIT_FAILURE);
            }
        } else
            // It's literal
            _sections[_currSectIndex]->write(temp->getValue(), _sections[_currSectIndex]->getLocCounter(), 4);

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
    auto instr = std::make_unique<Halt_Instr>();
    insertInstr(instr.get());
}

void Assembler::parseNoAdr(unsigned char inst) {
#ifdef LOG_PARSER
    auto name = static_cast<enum INSTRUCTION>(inst);
    std::cout << name << "\n";
#endif
    auto instr = std::make_unique<NoAdr_Instr>(static_cast<enum INSTRUCTION>(inst));
    insertInstr(instr.get());
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Jmp_Instr>(static_cast<enum INSTRUCTION>(inst), std::unique_ptr<Operand>(operand));
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand, RELOCATION::R_PC32);
}

void Assembler::addRelToInstr(Operand *operand, RELOCATION relType, uint32_t numBytesToFill) {
    auto symbol = findSymbol(operand->stringValue());
    if (symbol.first == -1) {
        _symbols.emplace_back(std::make_unique<Symbol>(
                operand->stringValue(),
                UNDEFINED,
                LOCAL,
                UNDEFINED,
                LABEL
        ));
    }
    _relocations.emplace_back(std::make_unique<Relocation>(
            operand->stringValue(),
            symbol.first == -1 ? findSymbol(operand->stringValue()).first : symbol.first,
            _txtSectIndex,
            _sections[_txtSectIndex]->getLocCounter() - numBytesToFill,
            numBytesToFill,
            relType
    ));
}

void Assembler::parseCall(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Call_Instr>(static_cast<enum INSTRUCTION>(inst), std::unique_ptr<Operand>(operand));
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand, RELOCATION::R_PC32);
}

void Assembler::parseCondJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<JmpCond_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                 std::unique_ptr<Operand>(operand));
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand, RELOCATION::R_PC32);
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "PUSH: %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Push_Instr>(gpr);
    insertInstr(instr.get());
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "POP: %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Pop_Instr>(gpr);
    insertInstr(instr.get());
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "NOT" << "\n";
#endif
    auto instr = std::make_unique<Not_Instr>(gpr);
    insertInstr(instr.get());
}

void Assembler::parseInt(Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    auto instr = std::make_unique<Int_Instr>(std::unique_ptr<Operand>(operand));
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand);
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << "XCHG: %r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    auto instr = std::make_unique<Xchg_Instr>(regS, regD);
    insertInstr(instr.get());
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << static_cast<enum INSTRUCTION>(inst) << ": ";
    std::cout << "%r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    auto instr = std::make_unique<TwoReg_Instr>(static_cast<enum INSTRUCTION>(inst), regS, regD);
    insertInstr(instr.get());
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %" << Csr::CSR[csr] << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    insertInstr(instr.get());
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << static_cast<int>(gpr) << ", %" << Csr::CSR[csr] << "\n";
#endif
    auto instr = std::make_unique<Csrwr_Instr>(gpr, csr);
    insertInstr(instr.get());
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
    if (operand->isCsr())
        operand = new IdentImm(operand->stringValue());
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    operand->log(std::cout);
    std::cout << ", %r" << static_cast<int>(gpr);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr);
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand);
}

void Assembler::parseLoad(unsigned char gpr1, unsigned char gpr2, int16_t offset) {
#ifdef LOG_PARSER
    std::cout << "LOAD: [%r" << static_cast<int>(gpr1) << "+" << offset << "], %r" << static_cast<int>(gpr2) << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(gpr1, gpr2, offset);
    insertInstr(instr.get());
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
    if (operand->isCsr())
        operand = new IdentImm(operand->stringValue());
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr) << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand));
    insertInstr(instr.get());
    if (operand->isLabel())
        addRelToInstr(operand);
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef LOG_PARSER
    std::cout << "GLOBAL: ";
    list->log(std::cout);
#endif
    SymbolList *temp = list;
    while (temp) {
        auto symbol = findSymbol(temp->_symbol);
        if (symbol.first == -1)
            _symbols.emplace_back(
                    std::make_unique<Symbol>(temp->_symbol, UNDEFINED, SCOPE::GLOBAL, 0, SYMBOL::LABEL));
        else
            symbolDuplicate(temp->_symbol);
        temp = temp->_next;
    }
    delete list;
}

void Assembler::log(std::ostream &out) const {
    logSymbols(out);
    logRelocations(out);
    logSections(out);
}

bool Assembler::hasUnresolvedSymbols() {
    for (auto &sym: _symbols)
        if (sym->core.flags.scope != GLOBAL && sym->core.sectionIndex == UNDEFINED)
            return true;
    return false;
}

void Assembler::logSections(std::ostream &out) const {
    Log::tableName(out, "Sections");
    Section::tableHeader(out);
    for (auto &sect: _sections)
        out << *sect;
    Log::tableFooter(out);
}

void Assembler::logRelocations(std::ostream &out) const {
    Log::tableName(out, "Relocations");
    Relocation::tableHeader(out);
    for (auto &rel: _relocations)
        out << *rel;
    Log::tableFooter(out);
}

void Assembler::logSymbols(std::ostream &out) const {
    Log::tableName(out, "Symbols");
    Symbol::tableHeader(out);
    for (auto &sym: _symbols)
        out << *sym;
    Log::tableFooter(out);
}

void Assembler::writeTxt() {
    std::ofstream out(_outputTxt);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open file " << _outputTxt << "\n";
        exit(EXIT_FAILURE);
    }
    log(out);
    Log::tableName(out, "Sections dump");
    for (auto &sect: _sections)
        sect->serialize(out);
    out.close();
}

void Assembler::writeObj() {
    std::ofstream out(_output, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open file " << _output << "\n";
        exit(EXIT_FAILURE);
    }

    // write num_symbols
    uint32_t num_symbols = _symbols.size();
    out.write((char *) &num_symbols, sizeof(uint32_t));

    // write Symbols: name_size, name, offset, sectionIndex, flags
    for (auto &sym: _symbols) {
        uint32_t name_size = sym->core.name.size();
        out.write((char *) &name_size, sizeof(uint32_t));
        out.write(sym->core.name.c_str(), name_size);
        uint64_t offset = sym->core.offset;
        out.write((char *) &offset, sizeof(uint64_t));
        uint64_t section = sym->core.sectionIndex;
        out.write((char *) &section, sizeof(uint64_t));
        out.write((char *) &sym->core.flags, sizeof(uint8_t));
    }

    // write num_sections
    uint32_t num_sections = _sections.size();
    out.write((char *) &num_sections, sizeof(uint32_t));

    // write Sections: name_size, name, data_size
    for (auto &sect: _sections) {
        uint32_t name_size = sect->core.name.size();
        out.write((char *) &name_size, sizeof(uint32_t));
        out.write(sect->core.name.c_str(), name_size);
        uint64_t data_size = sect->getSize();
        out.write((char *) &data_size, sizeof(uint64_t));
        out.write(reinterpret_cast<const char *>(sect->core.data.data()), data_size);
    }

    // write Relocations: symbol_index, section_index, offset, type
    uint32_t num_relocations = _relocations.size();
    out.write((char *) &num_relocations, sizeof(uint32_t));
    for (auto &rel: _relocations)
        out.write(reinterpret_cast<const char *>(&rel->_core), sizeof(rel->_core));
    out.close();
}

void Assembler::setOutput(char *str) {
    _outputTxt = _output = str; // ***.o
    _outputTxt.pop_back();
    _outputTxt.append("txt");
}

void Assembler::insertInstr(Instruction *instr) {
    if (_txtSectIndex == UNDEFINED) {
        _sections.emplace_back(std::make_unique<Section>(".text"));
        _txtSectIndex = _sections.size() - 1;
    }
    _sections[_txtSectIndex]->writeInstr((void *) &instr->bytes, _sections[_txtSectIndex]->getLocCounter());
}

std::pair<int32_t, Symbol *> Assembler::findSymbol(const std::string &symbol) {
    for (int i = 0; i < _symbols.size(); ++i)
        if (_symbols[i]->core.name == symbol)
            return {i, _symbols[i].get()};
    return {-1, nullptr};
}

void Assembler::symbolDuplicate(std::string &symbol) {
    std::cerr << "Error: Symbol " << symbol << " already defined." << "\n";
    exit(EXIT_FAILURE);
}

