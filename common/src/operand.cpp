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
        current = current->_next;
        if (current)
            out << ", ";
    }
}

Addressing WordLiteral::addRelocation(Assembler *as) {
    as->sections[as->currSection]->core.append(getValue(), 4);
    return {ADDR_UND, (int32_t) as->sections[as->currSection]->core.locationCnt()};
}

Addressing WordIdent::addRelocation(Assembler *as) {
    // If operand is label
    auto label = stringValue();
    auto symbol = as->findSymbol(label);

    if (symbol.index != -1) {
        // symbols defined in other files goes to relocations
        if (symbol.symbol->core.flags.source == OTHER) {
            auto relType = as->getRelocationType(symbol.symbol);
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

int64_t EquLiteral::getValue() {
    return _value;
}

void EquLiteral::logOne(std::ostream &out) {
    out << _value;
}

Addressing EquLiteral::addRelocation(Assembler *as) {
    throw std::runtime_error("EquLiteral::addRelocation() should not be used!");
}

EquResolved EquLiteral::tryToResolve(Assembler *as) {
    return {true, _value};
}

Addressing EquIdent::addRelocation(Assembler *as) {
    throw std::runtime_error("EquIdent::addRelocation() should not be used!");
}

int64_t EquIdent::getValue() {
    return 0;
}

void EquIdent::logOne(std::ostream &out) {
    out << ident;
}

std::string EquIdent::stringValue() {
    return ident;
}

EquResolved EquIdent::tryToResolve(Assembler *as) {
    auto symbol = as->findSymbol(ident);
    if (symbol.index != -1) {
        // Symbol is defined
        if (symbol.symbol->core.flags.defined) {
            auto sectionInd = symbol.symbol->core.sectionIndex;
            auto section = as->sections[sectionInd].get();
            auto ptr = section->core.data.data() + symbol.symbol->core.offset;
            return {true, *reinterpret_cast<int32_t *>(ptr)};
        } else if (symbol.symbol->core.flags.symbolType != NO_TYPE && symbol.symbol->core.flags.symbolType != EQU)
            // Symbol is defined, exit
            as->symbolDuplicate(symbol.symbol->core.name);
    } else
        // Symbol does not exist, add it
        as->declareSymbol(ident);
    return {false};
}

bool EquIdent::isResolved(Assembler *as) {
    auto symbol = as->findSymbol(ident);
    return symbol.index != -1 && as->isSymbolDefined(symbol.symbol);
}

void TwoReg::log(std::ostream &out) {
    out << "%r" << static_cast<int>(_gpr1);
    out << ", %r" << static_cast<int>(_gpr2);
}

void LiteralImm::log(std::ostream &out) {
    out << _value;
}

Addressing LiteralImm::addRelocation(Assembler *as) {
    if (fitIn12Bits(_value))
        return {REG_DIR, _value};
    as->addRelLiteral(_value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void LiteralInDir::log(std::ostream &out) {
    out << _value;
}

Addressing LiteralInDir::addRelocation(Assembler *as) {
    if (fitIn12Bits(_value))
        return {REG_DIR, _value};
    as->addRelLiteral(_value, 8);
    return {IN_DIR_IN_DIR, 0, GPR_TEMP};
}

void RegInDir::log(std::ostream &out) {
    out << "[%r" << _gpr << "]";
}

Addressing RegInDir::addRelocation(Assembler *as) {
    return {IN_DIR_OFFSET, 0, _gpr};
}

void RegDir::log(std::ostream &out) {
    out << "%r" << _gpr;
}

Addressing RegDir::addRelocation(Assembler *as) {
    return {REG_DIR, 0, _gpr};
}

void RegInDirOffLiteral::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _offset << "]";
}

Addressing RegInDirOffLiteral::addRelocation(Assembler *as) {
    if (!fitIn12Bits(_offset))
        throw std::runtime_error("Error: Offset " + std::to_string(_offset) + " is too big.");
    return {IN_DIR_INDEX, _offset};
}

void RegInDirOffIdent::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _ident << "]";
}

Addressing RegInDirOffIdent::addRelocation(Assembler *as) {
    auto symbol = as->findSymbol(_ident);
    if (symbol.index == -1)
        symbol = as->declareSymbol(_ident);
    as->addRelSymbol(symbol);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void CsrOp::log(std::ostream &out) {
    out << "%" << static_cast<REG_GPR>(_csr);
}

std::string CsrOp::stringValue() {
    throw std::runtime_error("CsrOp::stringValue() not implemented");
}

Addressing CsrOp::addRelocation(Assembler *as) {
    return {CSR_OP, 0, _csr};
}

void *WordLiteral::getValue() {
    return &_value;
}

void WordLiteral::logOne(std::ostream &out) {
    out << _value;
}

void *WordIdent::getValue() {
    return nullptr;
}

void WordIdent::logOne(std::ostream &out) {
    out << _ident;
}

std::string WordIdent::stringValue() {
    return _ident;
}

void IdentAddr::log(std::ostream &out) {
    out << _ident;
}

Addressing IdentAddr::addRelocation(Assembler *as) {
    auto symbol = as->findSymbol(_ident);
    int32_t value;
    if (symbol.index == -1)
        symbol = as->declareSymbol(_ident);
    if (as->isSymbolDefined(symbol.symbol)) {
        if (symbol.symbol->core.flags.symbolType == EQU) {
            value = symbol.symbol->core.offset;
            if (fitIn12Bits(value))
                return {REG_DIR, value};
            as->addRelLiteral(value, 4);
            return {IN_DIR_IN_DIR, 0, GPR_TEMP};
        }
        if (symbol.symbol->core.sectionIndex == as->currSection) {
            value = symbol.symbol->core.offset - as->sections[as->currSection]->core.locationCnt();
            if (fitIn12Bits(value))
                return {REG_DIR, value};
            as->addRelLiteral(value, 8);
            return {IN_DIR_IN_DIR, 0, GPR_TEMP};
        }
    }
    as->addRelSymbol(symbol, 4);
    return {IN_DIR_IN_DIR, 0, GPR_TEMP};
}

Addressing Operand::addRelocation(Assembler *as) {
    throw std::runtime_error("Operand::addRelocation() not implemented");
}
