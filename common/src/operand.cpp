#include "../include/symbol.h"
#include "../include/operand.h"
#include "../include/section.h"
#include "../include/relocation.h"
#include "../include/instruction.h"
#include "../../assembler/include/assembler.h"

#include <iostream>

void WordOperand::log(std::ostream &out) {
    WordOperand *current = this;
    while (current != nullptr) {
        current->logOne(out);
        current = current->next;
        if (current)
            out << ", ";
    }
}

Addressing WordLiteral::addRelocation(Assembler *as) {
    as->sections[as->currSection]->core.append(getValue(), 4);
    return {ADDR_UND, as->sections[as->currSection]->core.locationCnt()};
}

Addressing WordIdent::addRelocation(Assembler *as) {
    // If operand is label
    auto label = stringValue();
    auto symbol = as->findSymbol(label);

    if (symbol.index != -1) {
        // symbols defined in other files goes to relocations
        if (symbol.symbol->core.flags.source == OTHER) {
            auto relType = as->getRelocationType(symbol.symbol, true);
            as->relocations.emplace_back(std::make_unique<Relocation>(
                    symbol.symbol->core.name,
                    symbol.index,
                    as->currSection,
                    as->sections[as->currSection]->core.locationCnt(),
                    relType
            ));
            as->sections[as->currSection]->core.addToLocCounter(4);
        } else {
            // if symbol is not defined, add it to wordBackPatch
            if (!symbol.symbol->core.flags.defined) {
                auto addrToFill =
                        as->sections[as->currSection]->core.data.data() +
                        as->sections[as->currSection]->core.locationCnt();
                as->wordBackPatch[symbol.symbol].push_back(addrToFill);
                as->sections[as->currSection]->core.addToLocCounter(4);
            } else {
                // symbol is defined, writeAndIncr it
                void *ptr;
                // if not equ
                if (symbol.symbol->core.flags.symbolType != EQU) {
                    auto section = as->sections[symbol.symbol->core.sectionIndex].get();
                    ptr = section->core.data.data() + symbol.symbol->core.offset;
                } else
                    ptr = &symbol.symbol->core.offset;
                as->sections[as->currSection]->core.append(ptr, 4);
            }
        }
    } else
        throw std::runtime_error("Error: Symbol " + label + " have to be declared.");
    return {ADDR_UND, 0};
}

void EquOperand::log(std::ostream &out) {
    EquOperand *current = this;
    while (current != nullptr) {
        current->logOne(out);
        if (current->next)
            out << " " << current->op << " ";
        current = current->next;
    }
}

uint32_t EquLiteral::getValue() {
    return value;
}

void EquLiteral::logOne(std::ostream &out) {
    out << value;
}

Addressing EquLiteral::addRelocation(Assembler *as) {
    throw std::runtime_error("EquLiteral::addRelocation() should not be used!");
}

std::string EquLiteral::getIdent() {
    throw std::runtime_error("EquLiteral::getIdent() should not be used!");
}

bool EquLiteral::isLabel() {
    return false;
}

Addressing EquIdent::addRelocation(Assembler *as) {
    throw std::runtime_error("EquIdent::addRelocation() should not be used!");
}

uint32_t EquIdent::getValue() {
    throw std::runtime_error("EquIdent::getValue() should not be used!");
}

void EquIdent::logOne(std::ostream &out) {
    out << ident;
}

std::string EquIdent::stringValue() {
    return ident;
}

bool EquIdent::isResolved(Assembler *as) {
    auto indexSymbol = as->findSymbol(ident);
    return Assembler::isSymbolDefined(indexSymbol);
}

bool EquIdent::isLabel() {
    return true;
}

std::string EquIdent::getIdent() {
    return ident;
}

void LiteralImm::log(std::ostream &out) {
    out << value;
}

Addressing LiteralImm::addRelocation(Assembler *as) {
    if (fitIn12Bits(value))
        return {REG_DIR, value};
    as->addRelLiteral(value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void LiteralInDir::log(std::ostream &out) {
    out << value;
}

Addressing LiteralInDir::addRelocation(Assembler *as) {
    if (fitIn12Bits(value))
        return {IN_DIR_OFFSET, value};
    as->addRelLiteral(value, 4);
    return {IN_DIR_IN_DIR, 0, GPR_TEMP};
}

void RegInDir::log(std::ostream &out) {
    out << "[%r" << gpr << "]";
}

Addressing RegInDir::addRelocation(Assembler *as) {
    return {IN_DIR_OFFSET, 0, gpr};
}

void RegDir::log(std::ostream &out) {
    out << "%r" << gpr;
}

Addressing RegDir::addRelocation(Assembler *as) {
    return {REG_DIR, 0, gpr};
}

void RegInDirOffLiteral::log(std::ostream &out) {
    out << "[%r" << gpr << "+" << offset << "]";
}

Addressing RegInDirOffLiteral::addRelocation(Assembler *as) {
    if (!fitIn12Bits((int32_t) offset))
        displacementToBig((int32_t) offset);
    return {IN_DIR_INDEX, offset};
}

void RegInDirOffIdent::log(std::ostream &out) {
    out << "[%r" << gpr << "+" << ident << "]";
}

Addressing RegInDirOffIdent::addRelocation(Assembler *as) {
    auto indexSymbol = as->isSymbolDefined(ident);
    if (!Assembler::isSymbolDefined(indexSymbol))
        as->symbolNotDefined(ident);
    auto value = as->getValueOfSymbol(indexSymbol.symbol);
    if (!fitIn12Bits(value))
        displacementToBig((int32_t) value);
    return {REG_DIR, value, REG_PC};
}

void CsrOp::log(std::ostream &out) {
    out << "%" << static_cast<REG_GPR>(csr);
}

std::string CsrOp::stringValue() {
    throw std::runtime_error("CsrOp::stringValue() not implemented");
}

Addressing CsrOp::addRelocation(Assembler *as) {
    return {CSR_OP, 0, csr};
}

void *WordLiteral::getValue() {
    return &value;
}

void WordLiteral::logOne(std::ostream &out) {
    out << value;
}

void *WordIdent::getValue() {
    return nullptr;
}

void WordIdent::logOne(std::ostream &out) {
    out << ident;
}

std::string WordIdent::stringValue() {
    return ident;
}

void IdentImm::log(std::ostream &out) {
    out << "$" << ident;
}

Addressing Operand::addRelocation(Assembler *as) {
    throw std::runtime_error("Operand::addRelocation() not implemented");
}

void IdentInDir::log(std::ostream &out) {
    out << ident;
}

Addressing IdentImm::addRelocation(Assembler *as) {
    auto indexSymbol = as->findSymbol(ident);
    if (!as->isSymbolDeclared(indexSymbol))
        indexSymbol = as->declareSymbol(ident);
    else if (Assembler::isSymbolDefined(indexSymbol)) {
        uint32_t value = 0;
        if (indexSymbol.symbol->core.flags.symbolType == EQU)
            value = as->getValueOfSymbol(indexSymbol.symbol);
        else if (indexSymbol.symbol->core.sectionIndex == as->currSection)
            value = indexSymbol.symbol->core.offset - as->sections[as->currSection]->core.locationCnt();
        if (fitIn12Bits(value))
            return {REG_DIR, value};
        as->addRelLiteral(value);
        return {IN_DIR_OFFSET, 0, REG_PC};
    }
    as->addRelSymbol(indexSymbol);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

Addressing IdentInDir::addRelocation(Assembler *as) {
    auto indexSymbol = as->findSymbol(ident);
    if (!as->isSymbolDeclared(indexSymbol))
        indexSymbol = as->declareSymbol(ident);
    else if (Assembler::isSymbolDefined(indexSymbol)) {
        uint32_t value = 0;
        if (indexSymbol.symbol->core.flags.symbolType == EQU)
            value = as->getValueOfSymbol(indexSymbol.symbol);
        else if (indexSymbol.symbol->core.sectionIndex == as->currSection)
            value = indexSymbol.symbol->core.offset - as->sections[as->currSection]->core.locationCnt();
        if (fitIn12Bits(value))
            return {IN_DIR_OFFSET, value};
        as->addRelLiteral(value, 4);
        return {IN_DIR_IN_DIR, 0, GPR_TEMP};
    }
    as->addRelSymbol(indexSymbol, 4);
    return {IN_DIR_IN_DIR, 0, GPR_TEMP};
}

