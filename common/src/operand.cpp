#include "../include/operand.h"

#include <iostream>

std::string Csr::CSR[] = {"status", "handler", "cause"};

void WordOperand::logOne() {
}

void WordOperand::log() {
    WordOperand *current = this;
    while (current != nullptr) {
        current->logOne();
        current = current->_next;
        if (current)
            std::cout << ", ";
    }
}

void GprGprLiteral::log() {
    std::cout << "%r" << _gpr1;
    std::cout << ", %r" << _gpr2;
    std::cout << ", " << _value;
}

void GprGprIdent::log() {
    std::cout << "%r" + std::to_string(_gpr1);
    std::cout << ", %r" + std::to_string(_gpr2);
    std::cout << ", ";
    std::cout << _ident;
}

void LiteralImm::log() {
    std::cout << _value;
}

void LiteralInDir::log() {
    std::cout << _value;
}

void GprCsr::log() {
    std::cout << "%r" << _gpr;
    std::cout << ", %" + Csr::CSR[_csr];
}

void IdentDir::log() {
    std::cout << _ident;
}

void RegInDir::log() {
    std::cout << "%r" << _gpr;
}

void RegDir::log() {
    std::cout << "%r" << _gpr;
}

void RegInDirOffLiteral::log() {
    std::cout << "%r" << _gpr;
    std::cout << ", " << _offset;
}

void RegInDirOffIdent::log() {
    std::cout << "%r" << _gpr;
    std::cout << ", " << _ident;
}

void CsrOp::log() {
    std::cout << "%" << Csr::CSR[_csr];
}