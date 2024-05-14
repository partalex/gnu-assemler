#pragma once

#include <string>

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

    virtual void log() = 0;
};

class WordOperand : public Operand {
public:
    WordOperand *_next = nullptr;

    virtual ~WordOperand() { delete _next; }

    explicit WordOperand(WordOperand *next = nullptr) {
        _next = next;
    }

    void logOne();

    void log() override;
};

class WordLiteral : public WordOperand {
    unsigned int _value;
public:
    explicit WordLiteral(unsigned int value, WordOperand *next = nullptr) : _value(value) {
        _next = next;
    }

};

class WordIdent : public WordOperand {
    std::string _ident;
public:
    explicit WordIdent(std::string ident, WordOperand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }

};

class LiteralImm : public Operand {
public:
    explicit LiteralImm(unsigned int value) : _value(value) {}

    void log() override;

private:
    unsigned int _value;
};

class LiteralInDir : public Operand {
public:
    explicit LiteralInDir(unsigned int value) : _value(value) {}

    void log() override;

private:
    unsigned int _value;

};

class IdentDir : public Operand {
public:
    explicit IdentDir(std::string ident) : _ident(std::move(ident)) {}

    void log() override;

private:
    std::string _ident;
};

class RegInDir : public Operand {
public:
    explicit RegInDir(unsigned char gpr) : _gpr(gpr) {}

    void log() override;

private:
    unsigned char _gpr;
};

class RegDir : public Operand {
public:
    explicit RegDir(unsigned char gpr) : _gpr(gpr) {}

    void log() override;

private:
    unsigned char _gpr;
};

class RegInDirOffLiteral : public Operand {
public:
    explicit RegInDirOffLiteral(unsigned char gpr, unsigned int offset) : _gpr(gpr), _offset(offset) {}

    void log() override;

private:
    unsigned char _gpr;
    unsigned int _offset;
};

class RegInDirOffIdent : public Operand {
public:
    explicit RegInDirOffIdent(unsigned char gpr, std::string ident) : _gpr(gpr), _ident(std::move(ident)) {}

    void log() override;

private:
    unsigned char _gpr;
    std::string _ident;
};

class GprCsr : public Operand {
public:
    explicit GprCsr(unsigned char gpr, unsigned char csr) : _gpr(gpr), _csr(csr) {
    }

    void log() override;

private:
    unsigned char _gpr;
    unsigned char _csr;
};

class GprGprIdent : public Operand {
public:
    explicit GprGprIdent(unsigned char gpr1, unsigned char gpr2, std::string ident) : _gpr1(gpr1), _gpr2(gpr2),
                                                                                      _ident(std::move(ident)) {
    }

    void log() override;

private:
    unsigned char _gpr1;
    unsigned char _gpr2;
    std::string _ident;
};

class GprGprLiteral : public Operand {
public:
    explicit GprGprLiteral(unsigned char gpr1, unsigned char gpr2, unsigned int value) : _gpr1(gpr1), _gpr2(gpr2),
                                                                                         _value(value) {
    }

    void log() override;

private:
    unsigned char _gpr1;
    unsigned char _gpr2;
    unsigned int _value;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(unsigned char csr) : _csr(csr) {}

    void log() override;

private:
    unsigned char _csr;
};
