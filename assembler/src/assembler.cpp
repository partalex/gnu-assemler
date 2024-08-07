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
    as.input = argv[argc - 1];
    for (int i = 1; i < argc - 1; ++i)
        if (!strcmp(argv[i], "-o")) {
            as.setOutput(argv[++i]);
            break;
        }
    freopen(as.input.c_str(), "r", stdin);
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
        if (symbol.index == -1)
            symbols.emplace_back(
                    std::make_unique<Symbol>(
                            temp->_symbol,
                            UNDEFINED,
                            SCOPE::GLOBAL,
                            UNDEFINED,
                            SYMBOL::NO_TYPE,
                            OTHER,
                            NOT_DEFINED
                    ));
        else if (symbol.symbol->core.flags.scope == SCOPE::GLOBAL)
            throw std::runtime_error("Error: Symbol " + temp->_symbol + " already defined as global.");
        temp = temp->_next;
    }
    delete list;
}

void Assembler::parseSkip(int literal) {
#ifdef LOG_PARSER
    std::cout << "SKIP: " << literal << "\n";
#endif
    sections[currSection]->core.addToLocCounter(literal);
}

void Assembler::resolveEqu() {
    std::unordered_set<Symbol *> unresolvedEqu;
    for (auto &sym: symbols)
        if (!sym->core.flags.defined)
            if (sym->core.flags.symbolType == EQU)
                unresolvedEqu.insert(sym.get());
            else if (sym->core.flags.symbolType != NO_TYPE)
                throw std::runtime_error("Error: Symbol " + sym->core.name + " is not defined.");

    auto newResolved = true;
    while (newResolved) {
        newResolved = false;
        for (auto &symbol: unresolvedEqu) {
            auto expr = equExpr[symbol].get();
            int64_t value = 0;
            EquOperand *prev = nullptr;
            while (expr) {
                bool isAdd = prev == nullptr || prev->op == E_ADD;
                if (!expr->isLabel())
                    value += isAdd ? expr->getValue() : -expr->getValue();
                else {
                    auto sym1 = findSymbol(expr->stringValue());
                    if (sym1.symbol->core.flags.defined) {
                        auto increment = sym1.symbol->core.offset;
                        value += isAdd ? increment : -increment;
                    } else
                        break;
                }
                prev = expr;
                expr = expr->next;
            }
            if (!expr) {
                // set offset to value
                symbol->core.offset = value;
                symbol->core.flags.defined = true;
                unresolvedEqu.erase(symbol);
                newResolved = true;
                break;
            }
        }
    }
}

void Assembler::resolveWord() {
    for (auto &par: wordBackPatch) {
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
                auto section = sections[symbol->core.sectionIndex].get();
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
    correctRelocations();
    appendLiterals();
    writeTxt();
    writeObj();
}

void Assembler::parseLabel(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "LABEL: " << str << "\n";
#endif
    auto symbol = findSymbol(str);
    if (symbol.index != -1) {
        // Symbol is in symbols

        if (symbol.symbol->core.flags.symbolType != NO_TYPE)
            symbolDuplicate(symbol.symbol->core.name);

        // Set symbol
        symbol.symbol->core.offset = sections[currSection]->core.locationCnt();
        symbol.symbol->core.sectionIndex = currSection;
        symbol.symbol->core.flags.symbolType = SYMBOL::LABEL;
        symbol.symbol->core.flags.defined = DEFINED;
    } else {
        // Symbol does not exist, add it
        symbols.emplace_back(std::make_unique<Symbol>(
                str,
                currSection,
                SCOPE::LOCAL,
                sections[currSection]->core.locationCnt(),
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
    auto it = std::find_if(sections.begin(), sections.end(), [&](const auto &section) {
        return section->core.name == str;
    });
    if (it == sections.end()) {
        sections.emplace_back(std::make_unique<Section>(str));
        it = std::prev(sections.end());
    }
    currSection = it - sections.begin();
}

void Assembler::parseWord(WordOperand *operand) {
#ifdef LOG_PARSER
    std::cout << "WORD: ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto *temp = operand;
    while (temp) {
        temp->addRelocation(this);
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
    if (symbol.index != -1) {
        // Symbol is in symbols
        if (symbol.symbol->core.flags.symbolType == NO_TYPE) {
            symbol.symbol->core.flags.symbolType = EQU;
        } else if (symbol.symbol->core.flags.symbolType != EQU)
            // Symbol is not EQU then it's duplicate, exit
            symbolDuplicate(symbol.symbol->core.name);
    } else {
        // add symbol to symbols
        symbols.emplace_back(std::make_unique<Symbol>(
                str,
                MARKER::UNDEFINED,
                LOCAL,
                MARKER::UNDEFINED,
                EQU,
                THIS,
                NOT_DEFINED
        ));
    }

    symbol = findSymbol(str);
    equExpr[symbol.symbol] = std::unique_ptr<EquOperand>(operand);
    auto *temp = operand;
    auto canResolve = true;
    int32_t value = 0;
    while (temp) {
        auto resolveResult = temp->tryToResolve(this);
        if (resolveResult.resolved)
            value += resolveResult.value;
        else {
            canResolve = false;
            break;
        }
        temp = temp->next;
    }
    if (canResolve) {
        symbol.symbol->core.offset = value;
        symbol.symbol->core.flags.defined = DEFINED;
    }

}

void Assembler::parseAscii(const std::string &str) {
#ifdef LOG_PARSER
    std::cout << "ASCII: " << str << "\n";
#endif
    sections[currSection]->core.append((void *) str.c_str(), str.length());
}

void Assembler::parseHalt() {
#ifdef LOG_PARSER
    std::cout << "HALT" << "\n";
#endif
    auto instr = std::make_unique<Halt_Instr>();
    instr->insertInstr(this);
}

void Assembler::parseRet() {
#ifdef LOG_PARSER
    std::cout << "RET" << " \n";
#endif
    auto instr = std::make_unique<Pop_Instr>((uint8_t) REG_PC);
    instr->insertInstr(this);
}

void Assembler::parseIRet() {
#ifdef LOG_PARSER
    std::cout << "IRET" << " \n";
#endif
    auto instr = std::make_unique<IRet_Instr>();
    instr->insertInstr(this);
    // TODO
//    insertInstr(instr.get());
//    instr = std::make_unique<Pop_Instr>((uint8_t) CSR_STATUS);
//    instr->setInstr(CSR_LD_POST_INC);
//    insertInstr(instr.get());
}

void Assembler::parseJmp(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Jmp_Instr>(static_cast<enum INSTRUCTION>(inst),
                                             operand, this);
    instr->insertInstr(this);
}

void Assembler::parseCall(unsigned char inst, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Call_Instr>(static_cast<enum INSTRUCTION>(inst), operand, this);
    instr->insertInstr(this);
}

void Assembler::parsePush(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "PUSH: %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Push_Instr>(gpr);
    instr->insertInstr(this);
}

void Assembler::parsePop(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "POP: %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Pop_Instr>(gpr);
    instr->insertInstr(this);
}

void Assembler::parseNot(unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "NOT" << "\n";
#endif
    auto instr = std::make_unique<Not_Instr>(gpr);
    instr->insertInstr(this);
}

void Assembler::parseInt() {
#ifdef LOG_PARSER
    std::cout << "INT: ";
#endif
    auto instr = std::make_unique<Int_Instr>();
    instr->insertInstr(this);
}

void Assembler::parseXchg(unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << "XCHG: %r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    auto instr = std::make_unique<Xchg_Instr>(regS, regD);
    instr->insertInstr(this);
}

void Assembler::parseTwoReg(unsigned char inst, unsigned char regS, unsigned char regD) {
#ifdef LOG_PARSER
    std::cout << static_cast<enum INSTRUCTION>(inst) << ": ";
    std::cout << "%r" << (short) regS << ", %r" << (short) regD << "\n";
#endif
    auto instr = std::make_unique<TwoReg_Instr>(static_cast<enum INSTRUCTION>(inst), regS, regD);
    instr->insertInstr(this);
}

void Assembler::parseCsrrd(unsigned char csr, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %" << static_cast<REG_GPR>(csr) << ", %r" << static_cast<int>(gpr) << "\n";
#endif
    auto instr = std::make_unique<Csrrd_Instr>(csr, gpr);
    instr->insertInstr(this);
}

void Assembler::parseCsrwr(unsigned char gpr, unsigned char csr) {
#ifdef LOG_PARSER
    std::cout << "CSRRD: %r" << static_cast<int>(gpr) << ", %" << static_cast<REG_GPR>(csr) << "\n";
#endif
    auto instr = std::make_unique<Csrwr_Instr>(gpr, csr);
    instr->insertInstr(this);
}

void Assembler::parseLoad(Operand *operand, unsigned char gpr) {
#ifdef LOG_PARSER
    std::cout << "LOAD: ";
    operand->log(std::cout);
    std::cout << ", %r" << static_cast<int>(gpr);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(operand, gpr, this);
    instr->insertInstr(this);
}

void Assembler::parseLoad(unsigned char gpr1, int16_t offset, unsigned char gpr2) {
#ifdef LOG_PARSER
    std::cout << "LOAD: [%r" << static_cast<int>(gpr1) << "+" << offset << "], %r" << static_cast<int>(gpr2)
              << "\n";
#endif
    auto instr = std::make_unique<Load_Instr>(gpr1, gpr2, offset);
    instr->insertInstr(this);
}

void Assembler::parseStore(unsigned char gpr, Operand *operand) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr) << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<Store_Instr>(gpr, operand, this);
    instr->insertInstr(this);
}

void Assembler::parseGlobal(SymbolList *list) {
#ifdef LOG_PARSER
    std::cout << "GLOBAL: ";
    list->log(std::cout);
#endif
    SymbolList *temp = list;
    while (temp) {
        auto symbol = findSymbol(temp->_symbol);
        if (symbol.index == -1)
            symbols.emplace_back(
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
    for (auto &sym: symbols)
        if (!sym->core.flags.defined && sym->core.flags.source != OTHER)
            throw std::runtime_error("Error: Symbol " + sym->core.name + " is not defined.");
}

void Assembler::logSections(std::ostream &out) const {
    Log::tableName(out, "Sections");
    Section::tableHeader(out);
    for (auto &sect: sections)
        out << *sect;
    Log::tableFooter(out);
}

void Assembler::logRelocations(std::ostream &out) const {
    Log::tableName(out, "Relocations");
    Relocation::tableHeader(out);
    for (auto &rel: relocations)
        out << *rel;
    Log::tableFooter(out);
}

void Assembler::logSymbols(std::ostream &out) const {
    Log::tableName(out, "Symbols");
    Symbol::tableHeader(out);
    for (auto &sym: symbols)
        out << *sym;
    Log::tableFooter(out);
}

void Assembler::writeTxt() {
    std::ofstream out(outputTxt);
    if (!out.is_open())
        throw std::runtime_error("Error: Unable to open file " + outputTxt);
    log(out);
    Log::tableName(out, "Sections dump");
    for (auto &sect: sections)
        sect->serialize(out);
    out.close();
}

void Assembler::writeObj() {
    std::ofstream out(output, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Error: Unable to open file " + output);

    // writeAndIncr num_symbols
    uint32_t num_symbols = symbols.size();
    out.write((char *) &num_symbols, sizeof(uint32_t));

    // writeAndIncr Symbols: name_size, name, offset, sectionIndex, flags
    for (auto &sym: symbols) {
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
    uint32_t num_sections = sections.size();
    out.write((char *) &num_sections, sizeof(uint32_t));

    // writeAndIncr Sections: name_size, name, data_size
    for (auto &sect: sections) {
        uint32_t name_size = sect->core.name.size();
        out.write((char *) &name_size, sizeof(name_size));
        out.write(sect->core.name.c_str(), name_size);
        auto data_size = sect->getSize();
        out.write((char *) &data_size, sizeof(data_size));
        out.write(reinterpret_cast<const char *>(sect->core.data.data()), data_size);
    }

    // writeAndIncr Relocations: symbol_index, section_index, offset, type
    uint32_t num_relocations = relocations.size();
    out.write((char *) &num_relocations, sizeof(uint32_t));
    for (auto &rel: relocations)
        out.write(reinterpret_cast<const char *>(&rel->core), sizeof(rel->core));
    out.close();
}

void Assembler::setOutput(char *str) {
    outputTxt = output = linkerPath + str;
    outputTxt.pop_back();
    outputTxt.append("txt");
}

void Assembler::insertInstr(Instruction *instr) {
    sections[currSection]->appendInstr(&instr->bytes);
}

int32_t Assembler::getSymbolIndex(const std::string &symbol) const {
    for (int i = 0; i < symbols.size(); ++i)
        if (symbols[i]->core.name == symbol)
            return i;
    return -1;
}

IndexSymbol Assembler::findSymbol(const std::string &symbol) const {
    auto index = getSymbolIndex(symbol);
    if (index == -1)
        return {-1, nullptr};
    return {index, symbols[index].get()};
}

void Assembler::symbolDuplicate(const std::string &symbol) {
    throw std::runtime_error("Error: Symbol " + symbol + " already defined.");
}

IndexSymbol Assembler::declareSymbol(const std::string &ident) {
    symbols.emplace_back(std::make_unique<Symbol>(
            ident,
            UNDEFINED,
            LOCAL,
            UNDEFINED,
            NO_TYPE,
            THIS,
            NOT_DEFINED
    ));
    return {(int32_t) symbols.size() - 1, symbols.back().get()};
}

Relocation *Assembler::addRelLiteral(IndexSymbol symbol, int offset) {
    relocations.emplace_back(std::make_unique<Relocation>(
            "PC+" + symbol.symbol->core.name,
            getSymbolIndex(symbol.symbol->core.name),
            currSection,
            sections[currSection]->core.locationCnt() + DISPLACEMENT_SIZE_BYTES + offset,
            R_12b
    ));
    return relocations.back().get();
}

Relocation *Assembler::addRelSymbol(IndexSymbol symbol, int offsetLiteral) {
    addRelLiteral(symbol, offsetLiteral);
    auto rel = symbolAlreadyRelocated(symbol);
    if (rel)
        return rel;
    auto offset = sections[currSection]->addLabel(symbol.symbol->core.name);
    auto relType = getRelocationType(symbol.symbol);
    relocations.emplace_back(std::make_unique<Relocation>(
            symbol.symbol->core.name,
            getSymbolIndex(symbol.symbol->core.name),
            currSection,
            offset,
            relType
    ));
    return relocations.back().get();
}

Relocation *Assembler::addRelLiteral(int32_t value, int offsetLiteral) {
    auto &section = *sections[currSection].get();
    auto offset = section.addLiteral(value);
    relocations.emplace_back(std::make_unique<Relocation>(
            "PC+" + std::to_string(value),
            offset / 4,
            currSection,
            section.core.locationCnt() + DISPLACEMENT_SIZE_BYTES + offsetLiteral,
            R_12b
    ));
    return relocations.back().get();
}

void Assembler::correctRelocations() {
    for (auto &rel: relocations) {
        if (rel->core.type == R_32_UND) {
            rel->core.symbolIndex = getSymbolIndex(symbols[rel->core.symbolIndex]->core.name);
            rel->core.type = R_32b_LOCAL;
        }
        if (rel->core.type == R_32b_LOCAL) {
            auto symbol = symbols[rel->core.symbolIndex].get();
            auto value = getValueOfSymbol(symbol);
            auto section = sections[rel->core.sectionIndex].get()->literalsSection;
            section.write(&value, rel->core.offset, 4);
        }
    }
    for (auto &rel: relocations) {
        if (rel->core.type == R_12b) {
            auto section = sections[rel->core.sectionIndex].get();
            auto destPtr = section->core.data.data() + rel->core.offset;
            auto symbolIndex = rel->core.symbolIndex;
            int32_t symbolOffset = sections[rel->core.sectionIndex]->core.locationCnt()
                                   - (rel->core.offset - DISPLACEMENT_SIZE_BYTES) + JMP_OVER_LITERALS;
            for (auto &rel2: relocations)
                if (rel2->core.type == R_32b_LOCAL || rel2->core.type == R_32_GLOBAL)
                    if (rel2->core.symbolIndex == symbolIndex && rel2->core.sectionIndex == rel->core.sectionIndex) {
                        symbolOffset += (int32_t) rel2->core.offset;
                        break;
                    }
            writeDisplacement(destPtr, symbolOffset);
        }
    }
    relocations
            .erase(std::remove_if(
                           relocations.begin(),
                           relocations.end(),
                           [](const auto &rel) {
                               return rel->core.type == R_12b || rel->core.type == R_32b_LOCAL;
                           }),
                   relocations.end());
}

void Assembler::appendLiterals() {
    uint32_t literalsSize;
    for (auto &sect: sections) {
        literalsSize = sect->literalsSection.locationCnt();
        if (literalsSize > 0) {
            auto instr =
                    std::make_unique<Jmp_Instr>(JMP, new LiteralImm(literalsSize), this);
            sect->appendInstr(&instr->bytes);
            sect->appendLiterals();
        }
    }
}

bool Assembler::isSymbolDefined(Symbol *symbol) const {
    auto indexSymbol = findSymbol(symbol->core.name);
    if (indexSymbol.index == -1)
        return false;
    if (!symbol->core.flags.defined)
        return false;
    return true;
}

int32_t Assembler::getValueOfSymbol(Symbol *symbol) const {
    if (symbol->core.flags.symbolType == EQU)
        return (int32_t) symbol->core.offset;
    auto section = sections[symbol->core.sectionIndex].get();
    return section->readWord(symbol->core.offset);
}

Relocation *Assembler::symbolAlreadyRelocated(IndexSymbol symbol) const {
//        if ((rel->core.type == R_32b_LOCAL || rel->core.type == R_32_GLOBAL || rel->core.type == R_32_UND)
    for (auto &rel: relocations)
        if (rel->core.type != R_12b && rel->core.symbolIndex == symbol.index)
            return rel.get();
    return nullptr;
}

RELOCATION Assembler::getRelocationType(Symbol *symbol) const {
    if (symbol->core.flags.source == THIS)
        if (symbol->core.sectionIndex == UNDEFINED)
            return R_32_UND;
        else if (symbol->core.sectionIndex == currSection)
            return R_32b_LOCAL;
    return R_32_GLOBAL;
}

void Assembler::parseCondJmp(unsigned char inst, unsigned char regS, unsigned char regD, Operand *operand) {
#ifdef LOG_PARSER
    auto _inst = static_cast<enum INSTRUCTION>(inst);
    std::cout << _inst << ": " << "%r" << (short) regS << ", %r" << (short) regD << ", ";
    operand->log(std::cout);
    std::cout << "\n";
#endif
    auto instr = std::make_unique<JmpCond_Instr>(static_cast<enum INSTRUCTION>(inst),
                                                 regS, regD, operand, this);
    instr->insertInstr(this);
}

void Assembler::parseStore(unsigned char gpr1, unsigned char gpr2, int16_t offset) {
#ifdef LOG_PARSER
    std::cout << "STORE: %r" << static_cast<int>(gpr1) << ", [%r" << static_cast<int>(gpr2) << "+" << offset << "]\n";
#endif
    auto instr = std::make_unique<Store_Instr>(gpr1, gpr2, offset);
    instr->insertInstr(this);
}
