#include "../include/lexer.h"
#include "../include/assembler.h"
#include "../../common/include/log.h"
#include "../../common/include/symbol.h"
#include "../../common/include/section.h"
#include "../../common/include/relocation.h"
#include "../../common/include/symbol_list.h"
#include "../../common/include/instruction.h"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <unordered_set>

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
                    std::make_unique<Symbol>(
                            temp->_symbol,
                            UNDEFINED,
                            SCOPE::GLOBAL,
                            UNDEFINED,
                            SYMBOL::NO_TYPE,
                            OTHER,
                            NOT_DEFINED
                    ));
        else if (symbol.second->core.flags.scope == SCOPE::GLOBAL)
            throw std::runtime_error("Error: Symbol " + temp->_symbol + " already defined as global.");
        temp = temp->_next;
    }
    delete list;
}

void Assembler::parseSkip(int literal) {
#ifdef LOG_PARSER
    std::cout << "SKIP: " << literal << "\n";
#endif
    _sections[_currSection]->addToLocCounter(literal);
}

void Assembler::resolveEqu() {
    std::unordered_set<Symbol *> unresolvedEqu;
    for (auto &sym: _symbols)
        if (sym->core.flags.symbolType == NO_TYPE) {
            if (!(sym->core.sectionIndex == UNDEFINED && sym->core.offset == UNDEFINED))
                throw std::runtime_error("Error: Symbol " + sym->core.name + " is not defined.");
        } else if (sym->core.flags.symbolType == EQU) {
            unresolvedEqu.insert(sym.get());
            auto expr = _equExpr[sym.get()].get();
            if (expr->isLabel()) {
                auto symbol = findSymbol(expr->stringValue());
                if (symbol.second->core.offset == UNDEFINED)
                    unresolvedEqu.insert(symbol.second);
            }
        }

    if (_literalSection == -1)
        return;

    auto newResolved = true;
    auto literalsSection = literalSectionIndex();
    while (newResolved) {
        newResolved = false;
        for (auto &symbol: unresolvedEqu) {
            auto expr = _equExpr[symbol].get();
            int64_t value = 0;
            EquOperand *prev = nullptr;
            while (expr) {
                bool isAdd = prev == nullptr || prev->_op == E_ADD;
                if (!expr->isLabel())
                    value += isAdd ? expr->getValue() : -expr->getValue();
                else {
                    auto sym1 = findSymbol(expr->stringValue());
                    if (sym1.second->core.flags.defined) {
                        auto increment = _sections[literalsSection]->readWord(sym1.second->core.offset);
                        value += isAdd ? increment : -increment;
                    } else
                        break;
                }
                prev = expr;
                expr = expr->_next;
            }
            if (!expr) {
                _sections[literalsSection]->write((void *) &value, symbol->core.offset, 4);
                symbol->core.flags.defined = true;
                symbol->core.sectionIndex = literalsSection;
                unresolvedEqu.erase(symbol);
                newResolved = true;
                break;
            }
        }
    }
}

void Assembler::resolveWord() {
    for (auto &par: _wordBackPatch) {
        auto symbol = par.first;
        if (!symbol->core.flags.defined)
            throw std::runtime_error("Error: Symbol " + symbol->core.name + " is not defined.");
        auto list = par.second;
        for (auto &dest: list) {
            void *src;
            if (symbol->core.flags.symbolType == EQU) {
                // copy value of the symbol, value is symbol offset
                src = (char *) &symbol->core.offset;
            } else {
                auto section = _sections[symbol->core.sectionIndex].get();
                auto base = section->core.data.data();
                src = base + symbol->core.offset;
            }
            std::memcpy(dest, src, 4);
        }
    }
}

void Assembler::parseEnd() {
#ifdef LOG_PARSER
    std::cout << "END" << "\n";
#endif
    resolveEqu();
    resolveWord();
    writeTxt();
    checkUnresolvedSymbols();
    writeObj();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "LABEL: " << str << "\n";
#endif
    auto symbol = findSymbol(str);
    if (symbol.first != -1) {
        // Symbol is in symbols

        if (symbol.second->core.flags.symbolType != NO_TYPE)
            symbolDuplicate(symbol.second->core.name);

        // Set symbol
        symbol.second->core.offset = _sections[_currSection]->locCnt;
        symbol.second->core.sectionIndex = _currSection;
        symbol.second->core.flags.symbolType = SYMBOL::LABEL;
        symbol.second->core.flags.defined = DEFINED;
    } else {
        // Symbol does not exist, add it
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                _currSection,
                SCOPE::LOCAL,
                _sections[_currSection]->locCnt,
                SYMBOL::LABEL,
                THIS,
                DEFINED
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
    _currSection = it - _sections.begin();
}

void Assembler::parseWord(WordOperand *operand) {
#ifdef LOG_PARSER
    std::cout << "WORD: ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto *temp = operand;
    while (temp) {
        temp->addRelocation(*this);
        temp = temp->_next;
    }
    delete operand;
}

void Assembler::parseEqu(const std::string &str, EquOperand *operand) {
#ifdef LOG_PARSER
    std::cout << "EQU: " << str << " = ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    // Str is EQU symbol
    auto symbol = findSymbol(str);
    if (symbol.first != -1) {
        // Symbol is in symbols
        if (symbol.second->core.flags.symbolType == NO_TYPE) {
            auto literalsSection = literalSectionIndex();
            symbol.second->core.sectionIndex = literalsSection;
            symbol.second->core.offset = _sections[literalsSection]->locCnt;
            _sections[literalsSection]->addToLocCounter(4);
            symbol.second->core.flags.symbolType = EQU;
        } else if (symbol.second->core.flags.symbolType != EQU)
            // Symbol is not EQU then it's duplicate, exit
            symbolDuplicate(symbol.second->core.name);
    } else {
        auto literalsSection = literalSectionIndex();
        // add symbol to symbols
        _symbols.emplace_back(std::make_unique<Symbol>(
                str,
                literalsSection,
                LOCAL,
                _sections[literalsSection]->locCnt,
                EQU,
                THIS,
                NOT_DEFINED
        ));
        _sections[literalsSection]->addToLocCounter(4);
    }

    symbol = findSymbol(str);
    _equExpr[symbol.second] = std::unique_ptr<EquOperand>(operand);
    auto *temp = operand;
    auto canResolve = true;
    int32_t value = 0;
    while (temp) {
        auto resolveResult = temp->tryToResolve(*this);
        if (resolveResult.resolved)
            value += resolveResult.value;
        else {
            canResolve = false;
            break;
        }
        temp = temp->_next;
    }
    if (canResolve) {
        _sections[literalSectionIndex()]->write((void *) &value, symbol.second->core.offset, 4);
        symbol.second->core.flags.defined = DEFINED;
    }

}

void Assembler::parseAscii(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "ASCII: " << str << "\n";
#endif
    _sections[_currSection]->writeAndIncr((void *) str.c_str(), _sections[_currSection]->locCnt,
                                          str.length());
}

void Assembler::parseHalt() {
#ifdef LOG_PARSER
    std::cout << "HALT" << "\n";
#endif
    auto instr = std::make_unique<Halt_Instr>();
    insertInstr(instr.get());
}

void Assembler::parseRet() {
#ifdef LOG_PARSER
    std::cout << "RET" << " \n";
#endif
    auto instr = std::make_unique<Pop_Instr>((uint8_t) REG_PC);
    insertInstr(instr.get());
}

void Assembler::parseIRet() {
#ifdef LOG_PARSER
    std::cout << "IRET" << " \n";
#endif
    auto instr = std::make_unique<Pop_Instr>((uint8_t) REG_PC);
    insertInstr(instr.get());
    // pop status;
    instr = std::make_unique<Pop_Instr>((uint8_t) CSR_STATUS);
    instr->setInstr(CSR_LD_POST_INC);
    insertInstr(instr.get());
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Jmp_Instr>(static_cast<enum INSTRUCTION>(inst),
                                             std::unique_ptr<Operand>(operand));
    auto addressing = operand->addRelocation(*this);
    instr->setDisplacement(addressing.value);
    // MMMM==0b0000: pc<=gpr[A=PC]+D;
    // MMMM==0b1000: pc<=mem32[gpr[A=PC]+D];
    switch (addressing.addressing) {
        // will line below be executed?
        case REG_DIR:         // LiteralImm, IdentAddr
            instr->setMode(0b0000);
            break;
        case IN_DIR_OFFSET:   // LiteralImm
            instr->setMode(0b1000);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Jmp instruction.");
    }
    insertInstr(instr.get());
}

void Assembler::parseCall(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Call_Instr>(static_cast<enum INSTRUCTION>(inst),
                                              std::unique_ptr<Operand>(operand));
    auto addressing = operand->addRelocation(*this);
    instr->setDisplacement(addressing.value);
    // MMMM==0b0000: pc<=gpr[A=PC]+gpr[B=0]+D;
    // MMMM==0b0001: pc<=mem32[gpr[A=PC]+gpr[B=0]+D]; // not used
    switch (addressing.addressing) {
        case REG_DIR:         // LiteralImm
            instr->setMode(0b0000);
            instr->setDisplacement(addressing.value);
            break;
        case IN_DIR_OFFSET:// LiteralImm, IdentAddr
            instr->setMode(0b0001);
            // displacement will be set in by relocation
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Jmp instruction.");
    }
    insertInstr(instr.get());
}

void Assembler::parseCondJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr =
            std::make_unique<JmpCond_Instr>(
                    static_cast<enum INSTRUCTION>(inst),
                    std::unique_ptr<Operand>(operand)
            );
    auto addressing = operand->addRelocation(*this);
    // MMMM==0b0001: if (gpr[B] == gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b0010: if (gpr[B] != gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b0011: if (gpr[B] signed> gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b1001: if (gpr[B] == gpr[C]) pc<=mem32[gpr[A=PC]+D];
    // MMMM==0b1010: if (gpr[B] != gpr[C]) pc<=mem32[gpr[A=PC]+D];
    // MMMM==0b1011: if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A=PC]+D];
    switch (addressing.addressing) {
        case REG_DIR:     // GprGprLiteral, GprGprIdent
            instr->setDisplacement(addressing.value);
            instr->andMode(0b0111);
            break;
        case IN_DIR_OFFSET:   // GprGprLiteral
            instr->orMode(0b1000);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Jmp instruction.");
    }
    insertInstr(instr.get());
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

void Assembler::parseInt() {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    auto instr = std::make_unique<Int_Instr>();
    insertInstr(instr.get());
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
    std::cout << "CSRRD: %" << static_cast<REG_GPR>(csr) << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    insertInstr(instr.get());
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << static_cast<int>(gpr) << ", %" << static_cast<REG_GPR>(csr) << "\n";
#endif
    auto instr = std::make_unique<Csrwr_Instr>(gpr, csr);
    insertInstr(instr.get());
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    operand->log(std::cout);
    std::cout << ", %r" << static_cast<int>(gpr);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(std::unique_ptr<Operand>(operand), gpr);
    auto addressing = operand->addRelocation(*this);
    // MMMM==0b0000: gpr[A]<=csr[B];
    // MMMM==0b0001: gpr[A]<=gpr[B]+D;
    // MMMM==0b0010: gpr[A]<=mem32[gpr[B]+gpr[C=0]+D];
    // MMMM==0b0011: gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
    instr->setRegB(addressing.reg);
    instr->setDisplacement(addressing.value);
    switch (addressing.addressing) {
        case CSR_OP: // CsrOp
            instr->setMode(0b0000);
            break;
        case REG_DIR: // RegDir, LiteralImmReg, LiteralInDir, IdentImm
            instr->setMode(0b0001);
            break;
        case IN_DIR_OFFSET: // RegInDirOffIdent, RegInDir, LiteralImmReg, LiteralInDir, IdentAddr
            instr->setMode(0b0010);
            break;
        case IN_DIR_INDEX: // RegInDirOffLiteral
            instr->setMode(0b0011);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Load instruction.");
    }
    insertInstr(instr.get());
}

void Assembler::parseLoad(unsigned char gpr1, unsigned char gpr2, int16_t offset) {
#ifdef LOG_PARSER
    std::cout << "LOAD: [%r" << static_cast<int>(gpr1) << "+" << offset << "], %r" << static_cast<int>(gpr2)
              << "\n";
#endif
    if (!Instruction::fitIn12Bits(offset))
        throw std::runtime_error("Error: Offset " + std::to_string(offset) + " is too big.");
    auto instr = std::make_unique<Load_Instr>(gpr1, gpr2, offset);
    insertInstr(instr.get());
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr) << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Store_Instr>(gpr, std::unique_ptr<Operand>(operand));
    auto addressing = operand->addRelocation(*this);
    // MMMM==0b0000: mem32[gpr[A=PC]+gpr[B=0]+D]<=gpr[C=gpr];
    // MMMM==0b0010: mem32[mem32[gpr[A=PC]+gpr[B=0]+D]]<=gpr[C];
    // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
    instr->setRegA(addressing.reg);
    instr->setDisplacement(addressing.value);
    switch (addressing.addressing) {
        case REG_DIR:           // IdentImm, RegDir, LiteralImmReg, LiteralInDir
            // MMMM==0b0000: mem32[gpr[A=reg]+gpr[B=0]+D]<=gpr[C=gpr];
            instr->setMode(0b0000);
            break;
        case IN_DIR_OFFSET:     // RegInDirOffIdent, RegInDir, LiteralImmReg, LiteralInDir, IdentAddr
            // MMMM==0b0010: mem32[mem32[gpr[A=PC]+gpr[B=0]+D]]<=gpr[C];
            // st [PC+off], %r1;
            instr->setMode(0b0010);
            break;
        case IN_DIR_INDEX:      // RegInDirOffLiteral: // st [%r1+0], %r2
            // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
            instr->setMode(0b0001);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Store instruction.");
    }
    insertInstr(instr.get());
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
                    std::make_unique<Symbol>(
                            temp->_symbol,
                            UNDEFINED,
                            SCOPE::GLOBAL,
                            UNDEFINED,
                            SYMBOL::NO_TYPE,
                            THIS,
                            NOT_DEFINED
                    ));
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

void Assembler::checkUnresolvedSymbols() {
    for (auto &sym: _symbols)
        if (!sym->core.flags.defined && sym->core.flags.source != OTHER)
            throw std::runtime_error("Error: Symbol " + sym->core.name + " is not defined.");
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
    if (!out.is_open())
        throw std::runtime_error("Error: Unable to open file " + _outputTxt);
    log(out);
    Log::tableName(out, "Sections dump");
    for (auto &sect: _sections)
        sect->serialize(out);
    out.close();
}

void Assembler::writeObj() {
    std::ofstream out(_output, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Error: Unable to open file " + _output);

    // writeAndIncr num_symbols
    uint32_t num_symbols = _symbols.size();
    out.write((char *) &num_symbols, sizeof(uint32_t));

    // writeAndIncr Symbols: name_size, name, offset, sectionIndex, flags
    for (auto &sym: _symbols) {
        uint32_t name_size = sym->core.name.size();
        out.write((char *) &name_size, sizeof(uint32_t));
        out.write(sym->core.name.c_str(), name_size);
        auto offset = sym->core.offset;
        out.write((char *) &offset, sizeof(offset));
        auto section = sym->core.sectionIndex;
        out.write((char *) &section, sizeof(section));
        out.write((char *) &sym->core.flags, sizeof(sym->core.flags));
    }

    // writeAndIncr num_sections
    uint32_t num_sections = _sections.size();
    out.write((char *) &num_sections, sizeof(uint32_t));

    // writeAndIncr Sections: name_size, name, data_size
    for (auto &sect: _sections) {
        uint32_t name_size = sect->core.name.size();
        out.write((char *) &name_size, sizeof(name_size));
        out.write(sect->core.name.c_str(), name_size);
        auto data_size = sect->getSize();
        out.write((char *) &data_size, sizeof(data_size));
        out.write(reinterpret_cast<const char *>(sect->core.data.data()), data_size);
    }

    // writeAndIncr Relocations: symbol_index, section_index, offset, type
    uint32_t num_relocations = _relocations.size();
    out.write((char *) &num_relocations, sizeof(uint32_t));
    for (auto &rel: _relocations)
        out.write(reinterpret_cast<const char *>(&rel->_core), sizeof(rel->_core));
    out.close();
}

void Assembler::setOutput(char *str) {
    _outputTxt = _output = _linkerPath + str;
    _outputTxt.pop_back();
    _outputTxt.append("txt");
}

void Assembler::insertInstr(Instruction *instr) {
    _sections[_currSection]->writeInstr((void *) &instr->bytes, _sections[_currSection]->locCnt);
}

std::pair<int32_t, Symbol *> Assembler::findSymbol(const std::string &symbol) {
    for (int i = 0; i < _symbols.size(); ++i)
        if (_symbols[i]->core.name == symbol)
            return {i, _symbols[i].get()};
    return {-1, nullptr};
}

void Assembler::symbolDuplicate(const std::string &symbol) {
    throw std::runtime_error("Error: Symbol " + symbol + " already defined.");
}

int32_t &Assembler::literalSectionIndex() const {
    if (singleton()._literalSection == -1) {
        singleton()._sections.emplace_back(std::make_unique<Section>(".literals"));
        singleton()._literalSection = (int32_t) _sections.size() - 1;
    }
    return singleton()._literalSection;
}

uint32_t Assembler::addLiteralToPool(int32_t value) {
    auto index = literalSectionIndex();
    auto *ptr = (int32_t *) _sections[index]->core.data.data();
    for (int i = 0; i < _sections[index]->locCnt; i += 4)
        if (ptr[i] == value)
            return i;
    _sections[index]->writeAndIncr((void *) &value, _sections[index]->locCnt, 4);
    return _sections[index]->locCnt;
}

void Assembler::declareSymbol(const std::string &ident) {
    _symbols.emplace_back(std::make_unique<Symbol>(
            ident,
            UNDEFINED,
            LOCAL,
            UNDEFINED,
            NO_TYPE,
            THIS,
            NOT_DEFINED
    ));
}

void Assembler::addRelIdent(const std::string &ident) {
    _relocations.emplace_back(std::make_unique<Relocation>(
            ident,
            findSymbol(ident).first,
            _currSection,
            _sections[_currSection]->locCnt + 2,
            R_12b
    ));
}

void Assembler::addRelLiteral(int32_t value) {
    auto literalIndex = addLiteralToPool(value);
    auto literalSection = literalSectionIndex();
    _relocations.emplace_back(std::make_unique<Relocation>(
            "PC+" + std::to_string(value),
            literalIndex,
            literalSection,
            _sections[literalSection]->locCnt + 2,
            R_12b
    ));
}
