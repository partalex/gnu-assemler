#include "operand.hpp"
#include "log.hpp"

std::string Csr::CSR[] = {"status", "handler", "cause"};

void WordOperand::logOne() {
#ifdef LOG_PARSER
#endif
}

void WordOperand::log() {
#ifdef LOG_PARSER
    WordOperand *current = this;
    while (current != nullptr) {
        current->logOne();
        current = current->_next;
        if (current)
            Log::STRING_LN("");
    }
#endif
}

void GprGprLiteral::log() {
    Log::STRING("%r" + std::to_string(_gpr1));
    Log::STRING(", %r" + std::to_string(_gpr2));
    Log::STRING(", " + std::to_string(_value));
}

void GprGprIdent::log() {
    Log::STRING("%r" + std::to_string(_gpr1));
    Log::STRING(", %r" + std::to_string(_gpr2));
    Log::STRING(", ");
    Log::STRING(_ident);
}

void LiteralImm::log() {
    Log::STRING(std::to_string(_value));
}

void LiteralInDir::log() {
    Log::STRING(std::to_string(_value));
}

void GprCsr::log() {
    Log::STRING("%r" + std::to_string(_gpr));
    Log::STRING(", %" + Csr::CSR[_csr]);
}

void IdentDir::log() {
    Log::STRING(_ident);
}

void RegInDir::log() {
    Log::STRING("%r" + std::to_string(_gpr));
}

void RegDir::log() {
    Log::STRING("%r" + std::to_string(_gpr));
}

void RegInDirOffLiteral::log() {
    Log::STRING("%r" + std::to_string(_gpr));
    Log::STRING(", " + std::to_string(_offset));
}

void RegInDirOffIdent::log() {
    Log::STRING("%r" + std::to_string(_gpr));
    Log::STRING(", " + _ident);
}

void CsrOp::log() {
    Log::STRING("%" + Csr::CSR[_csr]);
}