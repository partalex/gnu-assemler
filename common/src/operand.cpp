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

std::pair<ADDRESSING, uint32_t> WordLiteral::addRelocation(Assembler &as) {
    as._sections[as._currSection]->writeAndIncr(getValue(), as._sections[as._currSection]->locCnt, 4);
    return {ADDR_UND, as._sections[as._currSection]->locCnt};
}

std::pair<ADDRESSING, uint32_t> WordIdent::addRelocation(Assembler &as) {
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

std::pair<ADDRESSING, uint32_t> EquLiteral::addRelocation(Assembler &as) {
    throw std::runtime_error("EquLiteral::addRelocation() should not be used!");
}

void EquLiteral::backPatch(Assembler &as) {
}

std::pair<ADDRESSING, uint32_t> EquIdent::addRelocation(Assembler &as) {
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

void EquIdent::backPatch(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first != -1) {
        // Symbol is in symbols
        if (symbol.second->core.flags.symbolType == GLOBAL)
            // Symbol is defined, exit
            as.symbolDuplicate(symbol.second->core.name);
        // if Local do nothing
    } else
        // Symbol does not exist, add it
        as.declareSymbol(_ident);
}

void TwoReg::log(std::ostream &out) {
    out << "%r" << static_cast<int>(_gpr1);
    out << ", %r" << static_cast<int>(_gpr2);
}

void GprGprLiteral::log(std::ostream &out) {
    TwoReg::log(out);
    out << ", " << _value;
}

std::pair<ADDRESSING, uint32_t> GprGprLiteral::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {IMMEDIATE_LITERAL_RO, _value};
    throw std::runtime_error("Error: Value " + std::to_string(_value) + " is too big.");
}

void GprGprIdent::log(std::ostream &out) {
    TwoReg::log(out);
    out << ", " << _ident;
}

std::pair<ADDRESSING, uint32_t> GprGprIdent::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {IMMEDIATE_SYMBOL, 0};
}

void LiteralImm::log(std::ostream &out) {
    out << _value;
}

std::pair<ADDRESSING, uint32_t> LiteralImm::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {IMMEDIATE_LITERAL_RO, _value};
    else
        as.addRelLiteral(_value);
    return {IMMEDIATE_LITERAL_PC, 0};
}

void LiteralInDir::log(std::ostream &out) {
    out << _value;
}

std::pair<ADDRESSING, uint32_t> LiteralInDir::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {INDIRECT_LITERAL_12bits, _value};
    else
        as.addRelLiteral(_value);
    return {INDIRECT_LITERAL, 0};
}

void GprCsr::log(std::ostream &out) {
    out << "[%r" << _gpr << "+$" << static_cast<REGISTERS>(_csr) << "]";
}

std::pair<ADDRESSING, uint32_t> GprCsr::addRelocation(Assembler &as) {
    return {GRP_CSR, _csr};
}

void IdentImm::log(std::ostream &out) {
    out << "$" << _ident;
}

std::pair<ADDRESSING, uint32_t> IdentImm::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {IMMEDIATE_SYMBOL, 0};
}

void RegInDir::log(std::ostream &out) {
    out << "[%r" << _gpr << "]";
}

std::pair<ADDRESSING, uint32_t> RegInDir::addRelocation(Assembler &as) {
    return {INDIRECT_REGISTER, _gpr};
}

void RegDir::log(std::ostream &out) {
    out << "%r" << _gpr;
}

std::pair<ADDRESSING, uint32_t> RegDir::addRelocation(Assembler &as) {
    return {DIRECT_REGISTER, _gpr};
}

void RegInDirOffLiteral::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _offset << "]";
}

std::pair<ADDRESSING, uint32_t> RegInDirOffLiteral::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_offset);
    if (!fitIn12Bits)
        throw std::runtime_error("Error: Offset " + std::to_string(_offset) + " is too big.");
    return {INDIRECT_REG_LITERAL, _offset};
}

void RegInDirOffIdent::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _ident << "]";
}

std::pair<ADDRESSING, uint32_t> RegInDirOffIdent::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {INDIRECT_REG_SYMBOL, 0};
}

void CsrOp::log(std::ostream &out) {
    out << "%" << static_cast<REGISTERS>(_csr);
}

std::string CsrOp::stringValue() {
    throw std::runtime_error("CsrOp::stringValue() not implemented");
}

std::pair<ADDRESSING, uint32_t> CsrOp::addRelocation(Assembler &as) {
    return {CSR_OP, _csr};
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

std::pair<ADDRESSING, uint32_t> IdentAddr::addRelocation(Assembler &as) {
    auto symbol = as.findSymbol(_ident);
    if (symbol.first == -1)
        as.declareSymbol(_ident);
    as.addRelIdent(_ident);
    return {INDIRECT_SYMBOL, 0};
}

void LiteralImmReg::log(std::ostream &out) {
    out << "$" << _value;
}

std::pair<ADDRESSING, uint32_t> LiteralImmReg::addRelocation(Assembler &as) {
    auto fitIn12Bits = Instruction::fitIn12Bits(_value);
    if (fitIn12Bits)
        return {IMMEDIATE_LITERAL_RO, _value};
    else
        as.addRelLiteral(_value);
    return {IMMEDIATE_LITERAL_PC, 0};
}

std::pair<ADDRESSING, uint32_t> Operand::addRelocation(Assembler &as) {
    throw std::runtime_error("Operand::addRelocation() not implemented");
}

