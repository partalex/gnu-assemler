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

    void parseSection(const std::string &);

    void parseWord(Operand *);

    void parseAscii(const std::string &);

    void parseEnd();

    void parseHalt();

    void parseNoAdr();

    void parseJmp(Operand *);

    void parsePush(unsigned char);

    void parsePop(unsigned char);

    void parseNot(unsigned char);

    void parseInt(Operand *);

    void parseXchg(Operand *);

    void parseTwoReg(Operand *);

    void parseCsrrd(Operand *);

    void parseCsrwr(Operand *);

    void parseLoad(Operand *);

    void parseStore(Operand *);

};
