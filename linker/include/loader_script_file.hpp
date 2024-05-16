#include <iostream>
#include <vector>
#include <unordered_map>


class LoaderScriptFile {
public:

    void LoadFromFile(std::istream &);

    void
    FillSymbolsAndSectionPositions(std::unordered_map<std::string, Symbol> &, std::unordered_map<std::string, int> &);

    //unordered_map<string, Symbol> symbols;

    std::vector<std::string> scriptInput;
};

