#include "../include/operand.h"

#include <iostream>

std::string Csr::CSR[] = {"status", "handler", "cause"};

void WordOperand::log(std::ostream & out) {
    WordOperand *current = this;
    while (current != nullptr) {
        current->logOne(out);
        current = current->_next;
        if (current)
            out << ", ";
    }
}

void GprGprLiteral::log(std::ostream & out) {
    out << "%r" << _gpr1;
    out << ", %r" << _gpr2;
    out << ", " << _value;
}

void GprGprIdent::log(std::ostream & out) {
    out << "%r" + std::to_string(_gpr1);
    out << ", %r" + std::to_string(_gpr2);
    out << ", ";
    out << _ident;
}

void LiteralImm::log(std::ostream & out) {
    out << _value;
}

void LiteralInDir::log(std::ostream & out) {
    out << _value;
}

void GprCsr::log(std::ostream & out) {
    out << "%r" << _gpr;
    out << ", %" + Csr::CSR[_csr];
}

void IdentDir::log(std::ostream & out) {
    out << _ident;
}

void RegInDir::log(std::ostream & out) {
    out << "%r" << _gpr;
}

void RegDir::log(std::ostream & out) {
    out << "%r" << _gpr;
}

void RegInDirOffLiteral::log(std::ostream & out) {
    out << "%r" << _gpr;
    out << ", " << _offset;
}

void RegInDirOffIdent::log(std::ostream & out) {
    out << "%r" << _gpr;
    out << ", " << _ident;
}

void CsrOp::log(std::ostream & out) {
    out << "%" << Csr::CSR[_csr];
}

void *WordLiteral::getValue() {
    return &_value;
}

void WordLiteral::logOne(std::ostream &out) {
    out << _value;
}

void *WordIdent::getValue() {
    // TODO
    throw std::runtime_error("Needed to get value from table symbol");
}

void WordIdent::logOne(std::ostream &out) {
    out << _ident;
}
