#include "../include/operand.h"

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

void EquOperand::log(std::ostream &out) {
    EquOperand *current = this;
    while (current != nullptr) {
        current->logOne(out);
        if (current->_next)
            out << " " << current->_op << " ";
        current = current->_next;
    }
}

void GprGprLiteral::log(std::ostream &out) {
    out << "%r" << _gpr1;
    out << ", %r" << _gpr2;
    out << ", " << _value;
}

void GprGprIdent::log(std::ostream &out) {
    out << "%r" + std::to_string(_gpr1);
    out << ", %r" + std::to_string(_gpr2);
    out << ", ";
    out << _ident;
}

void LiteralImm::log(std::ostream &out) {
    out << _value;
}

void LiteralInDir::log(std::ostream &out) {
    out << _value;
}

void GprCsr::log(std::ostream &out) {
    out << "[%r" << _gpr << "+$" << static_cast<CSR>(_csr) << "]";
}

void IdentImm::log(std::ostream &out) {
    out << "$" << _ident;
}

void RegInDir::log(std::ostream &out) {
    out << "[%r" << _gpr << "]";
}

void RegDir::log(std::ostream &out) {
    out << "%r" << _gpr;
}

void RegInDirOffLiteral::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _offset << "]";
}

void RegInDirOffIdent::log(std::ostream &out) {
    out << "[%r" << _gpr << "+" << _ident << "]";
}

void CsrOp::log(std::ostream &out) {
    out << "%" << static_cast<CSR>(_csr);
}

int64_t EquLiteral::getValue() {
    return _value;
}

void EquLiteral::logOne(std::ostream &out) {
    out << _value;
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

void LiteralImmReg::log(std::ostream &out) {
    out << "$" << _value;
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
