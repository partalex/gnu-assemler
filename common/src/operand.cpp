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

Addressing WordLiteral::addRelocation(Assembler &as) {
    as._sections[as._currSection]->writeAndIncr(getValue(), as._sections[as._currSection]->locCnt, 4);
    return {ADDR_UND, (int32_t) as._sections[as._currSection]->locCnt};
}

Addressing WordIdent::addRelocation(Assembler &as) {
    // If operand is label
    auto label = stringValue();
    auto symbol = as.findSymbol(label);

    if (symbol.first != -1) {
        // symbols defined in other files goes to relocations
        if (symbol.second->core.flags.source == OTHER) {
            as._relocations.emplace_back(std::make_unique<Relocation>(
                    symbol.second->core.name,
                    symbol.first,
                    as._currSection,
                    as._sections[as._currSection]->locCnt,
                    4,
                    RELOCATION::R_WORD
            ));
            as._sections[as._currSection]->addToLocCounter(4);
        } else {
            // if symbol is not defined, add it to wordBackPatch
            if (!symbol.second->core.flags.defined) {
                auto addrToFill =
                        as._sections[as._currSection]->core.data.data() + as._sections[as._currSection]->locCnt;
                as._wordBackPatch[symbol.second].push_back(addrToFill);
                as._sections[as._currSection]->addToLocCounter(4);
            } else {
                // symbol is defined, writeAndIncr it
                auto section = as._sections[symbol.second->core.sectionIndex].get();
                auto ptr = section->core.data.data() + symbol.second->core.offset;
                as._sections[as._currSection]->writeAndIncr(ptr, as._sections[as._currSection]->locCnt, 4);
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
        if (current->_next)
            out << " " << current->_op << " ";
        current = current->_next;
    }
}

int64_t EquLiteral::getValue() {
    return _value;
}

void EquLiteral::logOne(std::ostream &out) {
    out << _value;
}

Addressing EquLiteral::addRelocation(Assembler &as) {
    throw std::runtime_error("EquLiteral::addRelocation() should not be used!");
}

EquResolved EquLiteral::tryToResolve(Assembler &as) {
    return {true, _value};
}

Addressing EquIdent::addRelocation(Assembler &as) {
    throw std::runtime_error("EquIdent::addRelocation() should not be used!");
}

int64_t EquIdent::getValue() {
    return 0;
}

void EquIdent::logOne(std::ostream &out) {
    out << _ident;
}

std::string EquIdent::stringValue() {
    return _ident;
}

EquResolved EquIdent::tryToResolve(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first != -1) {
        // Symbol is defined
         if(symbol.second->core.flags.defined){
            auto sectionInd = symbol.second->core.sectionIndex;
            auto section = as._sections[sectionInd].get();
            auto ptr = section->core.data.data() + symbol.second->core.offset;
            return {true, *reinterpret_cast<int32_t *>(ptr)};
        }
        else if (symbol.second->core.flags.symbolType != NO_TYPE && symbol.second->core.flags.symbolType != EQU)
            // Symbol is defined, exit
            as.symbolDuplicate(symbol.second->core.name);
    } else
        // Symbol does not exist, add it
        as.declareSymbol(_ident);
    return {false};
}

void TwoReg::log(std::ostream &out) {
    out << "%r" << static_cast<int>(_gpr1);
    out << ", %r" << static_cast<int>(_gpr2);
}

void GprGprLiteral::log(std::ostream &out) {
    TwoReg::log(out);
    out << ", " << _value;
}

Addressing GprGprLiteral::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {REG_DIR, _value};
    as.addRelLiteral(_value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

GprGprLiteral::GprGprLiteral(uint8_t gpr1, uint8_t gpr2, int32_t offset) :
        TwoReg(gpr1, gpr2), _value(offset) {
}

void GprGprIdent::log(std::ostream &out) {
    TwoReg::log(out);
    out << ", " << _ident;
}

Addressing GprGprIdent::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {REG_DIR, 0, REG_PC};
}

void LiteralImm::log(std::ostream &out) {
    out << _value;
}

Addressing LiteralImm::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {REG_DIR, _value};
    as.addRelLiteral(_value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void LiteralInDir::log(std::ostream &out) {
    out << _value;
}

Addressing LiteralInDir::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {REG_DIR, _value};
    as.addRelLiteral(_value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void IdentImm::log(std::ostream &out) {
    out << "$" << _ident;
}

Addressing IdentImm::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {REG_DIR, 0, REG_PC};
}

void RegInDir::log(std::ostream &out) {
    out << "[%r" << _gpr << "]";
}

Addressing RegInDir::addRelocation(Assembler &as) {
    return {IN_DIR_OFFSET, 0, _gpr};
}

void RegDir::log(std::ostream &out) {
    out << "%r" << _gpr;
}

Addressing RegDir::addRelocation(Assembler &as) {
    return {REG_DIR, 0, _gpr};
}

void RegInDirOffLiteral::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _offset << "]";
}

Addressing RegInDirOffLiteral::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_offset);
    if (!fitIn12Bits)
        throw std::runtime_error("Error: Offset " + std::to_string(_offset) + " is too big.");
    return {IN_DIR_INDEX, _offset};
}

void RegInDirOffIdent::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _ident << "]";
}

Addressing RegInDirOffIdent::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void CsrOp::log(std::ostream &out) {
    out << "%" << static_cast<REG_GPR>(_csr);
}

std::string CsrOp::stringValue() {
    throw std::runtime_error("CsrOp::stringValue() not implemented");
}

Addressing CsrOp::addRelocation(Assembler &as) {
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

Addressing IdentAddr::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

void LiteralImmReg::log(std::ostream &out) {
    out << "$" << _value;
}

Addressing LiteralImmReg::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {REG_DIR, _value};
    as.addRelLiteral(_value);
    return {IN_DIR_OFFSET, 0, REG_PC};
}

Addressing Operand::addRelocation(Assembler &as) {
    throw std::runtime_error("Operand::addRelocation() not implemented");
}
