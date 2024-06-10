#pragma once

#include <string>
#include <cstdint>

class Csr {
public:
    static std::string CSR[];
    enum CSR {
        STATUS,
        HANDLER,
        CAUSE
    };
};

class Operand {
public:

    explicit Operand() = default;

    virtual void log(std::ostream &) = 0;
};

class WordOperand : public Operand {
public:
    WordOperand *_next = nullptr;

    virtual ~WordOperand() { delete _next; }

    explicit WordOperand(WordOperand *next = nullptr) {
        _next = next;
    }

    virtual void logOne(std::ostream &out) = 0;

    void log(std::ostream &) override;

    virtual void *getValue() = 0;
};

class WordLiteral : public WordOperand {
    uint32_t _value;
public:
    explicit WordLiteral(uint32_t value, WordOperand *next = nullptr) : _value(value) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &out) override;

};

class WordIdent : public WordOperand {
    std::string _ident;
public:
    explicit WordIdent(std::string ident, WordOperand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &out) override;

};

class LiteralImm : public Operand {
public:
    explicit LiteralImm(uint32_t value) : _value(value) {}

    void log(std::ostream &) override;

private:
    uint32_t _value;
};

class LiteralInDir : public Operand {
public:
    explicit LiteralInDir(uint32_t value) : _value(value) {}

    void log(std::ostream &) override;

private:
    uint32_t _value;

};

class IdentDir : public Operand {
public:
    explicit IdentDir(std::string ident) : _ident(std::move(ident)) {}

    void log(std::ostream &) override;

private:
    std::string _ident;
};

class RegInDir : public Operand {
public:
    explicit RegInDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
};

class RegDir : public Operand {
public:
    explicit RegDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
};

class RegInDirOffLiteral : public Operand {
public:
    explicit RegInDirOffLiteral(uint8_t gpr, uint32_t offset) : _gpr(gpr), _offset(offset) {}

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
    uint32_t _offset;
};

class RegInDirOffIdent : public Operand {
public:
    explicit RegInDirOffIdent(uint8_t gpr, std::string ident) : _gpr(gpr), _ident(std::move(ident)) {}

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
    std::string _ident;
};

class GprCsr : public Operand {
public:
    explicit GprCsr(uint8_t gpr, uint8_t csr) : _gpr(gpr), _csr(csr) {
    }

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
    uint8_t _csr;
};

class GprGprIdent : public Operand {
public:
    explicit GprGprIdent(uint8_t gpr1, uint8_t gpr2, std::string ident) : _gpr1(gpr1), _gpr2(gpr2),
                                                                          _ident(std::move(ident)) {
    }

    void log(std::ostream &) override;

private:
    uint8_t _gpr1;
    uint8_t _gpr2;
    std::string _ident;
};

class GprGprLiteral : public Operand {
public:
    explicit GprGprLiteral(uint8_t gpr1, uint8_t gpr2, uint32_t value) : _gpr1(gpr1), _gpr2(gpr2),
                                                                         _value(value) {
    }

    void log(std::ostream &) override;

private:
    uint8_t _gpr1;
    uint8_t _gpr2;
    uint32_t _value;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(uint8_t csr) : _csr(csr) {}

    void log(std::ostream &) override;

private:
    uint8_t _csr;
};
