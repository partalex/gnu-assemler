#include <memory>
#include <vector>
#include "structures.hpp"

class Assembler {
    static std::shared_ptr<Assembler> _instance;
    std::string _output = "output/obj.o";
    std::string _input;
    std::vector<std::string> _sections;
    std::vector<std::string> _labels;

public:
    ~Assembler() = default;
    void operator=(Assembler const &) = delete;
    static Assembler &singleton();
    static int pass(int, char **);

    void parseLabel(const std::string &);
    void parseEnd();
    void parseGlobal(SymbolList *);
    void parseExtern(SymbolList *);
    void parseSection(const std::string &);
    void parseWord(Operand *);
    void parseAscii(const std::string &);
    void parseSkip(int);

    // instructions
    void parseHalt();
    void parseInt(Operand *);
    void parseJmp(Operand *);
    void parsePush(unsigned char);
    void parsePop(unsigned char);
    void parseNot(unsigned char);
    void parseXchg(unsigned char, unsigned char);
    void parseTwoReg(unsigned char, unsigned char, unsigned char);
    void parseCsrrd(unsigned char, unsigned char);
    void parseCsrwr(unsigned char, unsigned char);
    void parseLoad(Operand *, unsigned char);
    void parseStore(unsigned char, Operand *);
    void parseNoAdr(unsigned char);
};
