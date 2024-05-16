
#include <vector>
#include <unordered_map>
#include <string>

class ObjectFile {
public:

    void LoadFromFile(std::string );

    std::unordered_map<std::string,Section> sections;
    std::unordered_map<std::string,Symbol> symbols;
    std::vector<Relocation> relocations;


};
