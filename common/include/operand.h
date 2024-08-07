#pragma once

#include "enum.h"

#include <string>
#include <cstdint>
#include <optional>
#include <sstream>

class Assembler;

class Operand {
public:

    explicit Operand() = default;

    virtual void log(std::ostream &out) {}

    virtual std::string stringValue() {
        throw std::runtime_error("Operand::stringValue() not implemented");
    }

    virtual Addressing addRelocation(Assembler *);
};

class WordOperand : public Operand {
public:
    WordOperand *_next = nullptr;

    virtual ~WordOperand() { delete _next; }

    explicit WordOperand(WordOperand *next = nullptr) : _next(next) {
    }

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual void *getValue() = 0;

};

class WordLiteral : public WordOperand {
    int32_t _value;
public:
    explicit WordLiteral(int32_t value, WordOperand *next = nullptr) : _value(value) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;
};

class WordIdent : public WordOperand {
    std::string _ident;
public:
    explicit WordIdent(std::string ident, WordOperand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    Addressing addRelocation(Assembler *) override;

};

class EquOperand : public Operand {
public:
    EQU_OP op;
    EquOperand *next = nullptr;

    virtual ~EquOperand() { delete next; }

    explicit EquOperand(unsigned char op = E_ADD, EquOperand *next = nullptr)
            : op(static_cast<EQU_OP>(op)), next(next) {}

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual EquResolved tryToResolve(Assembler *) = 0;

    virtual bool isLabel() = 0;

    virtual bool isResolved(Assembler *) = 0;

    virtual int64_t getValue() { return 0; }
};

class EquLiteral : public EquOperand {
    int32_t _value;
public:
    explicit EquLiteral(int32_t value, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : _value(value), EquOperand(op, next) {}

    int64_t getValue() override;

    void logOne(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

    EquResolved tryToResolve(Assembler *) override;

    bool isResolved(Assembler *) override { return true; }

    bool isLabel() override { return false; }

};

class EquIdent : public EquOperand {
    std::string ident;
public:
    explicit EquIdent(std::string ident, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : ident(std::move(ident)), EquOperand(op, next) {}

    int64_t getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    Addressing addRelocation(Assembler *) override;

    bool isLabel() override { return true; }

    bool isResolved(Assembler *) override;

    EquResolved tryToResolve(Assembler *) override;
};

class LiteralImm : public Operand {
public:
    explicit LiteralImm(int32_t value) : _value(value) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    int32_t _value;
};

class LiteralInDir : public Operand {
public:
    explicit LiteralInDir(int32_t value) : _value(value) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    int32_t _value;
};

class IdentAddr : public Operand {
public:
    explicit IdentAddr(std::string ident) :
            _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::string stringValue() override { return _ident; }

    Addressing addRelocation(Assembler *) override;

private:
    std::string _ident;
};

class RegInDir : public Operand {
public:
    explicit RegInDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t _gpr;
};

class RegDir : public Operand {
public:
    explicit RegDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t _gpr;
};

class RegInDirOffLiteral : public Operand {
public:
    explicit RegInDirOffLiteral(uint8_t gpr, int32_t offset) : _gpr(gpr), _offset(offset) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t _gpr;
    int32_t _offset;
};

class RegInDirOffIdent : public Operand {
public:
    explicit RegInDirOffIdent(uint8_t gpr, std::string ident) : _gpr(gpr), _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;


    std::string stringValue() override { return _ident; }

private:
    uint8_t _gpr;
    std::string _ident;
};

class TwoReg : public Operand {
public:
    explicit TwoReg(uint8_t gpr1, uint8_t gpr2) :
            _gpr1(gpr1), _gpr2(gpr2) {}

    void log(std::ostream &) override;

    uint8_t getGpr1() const { return _gpr1; }

    uint8_t getGpr2() const { return _gpr2; }

private:
    uint8_t _gpr1;
    uint8_t _gpr2;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(uint8_t csr) : _csr(csr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

    std::string stringValue() override;

private:
    uint8_t _csr;
};
