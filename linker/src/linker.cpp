#include "../include/linker.h"

#include <regex>
#include <iostream>
#include <iomanip>

std::ofstream Linker::logFile("linker/log");

std::regex sectionRegex("^.(text|data|bss)(.[a-zA-Z_][a-zA-Z0-9]*)?$");

void Linker::Link(std::ifstream &loaderScriptFile, std::vector<std::string> &inputFiles, std::ofstream &outputFile) {
    try {

        logFile << "Load script file" << std::endl;
        loaderScript.LoadFromFile(loaderScriptFile);


        for (auto &inputFile: inputFiles) {
            logFile << "Linking file " << std::endl;
            LoadFile(inputFile);
        }


        logFile << "Load symbols from script" << std::endl;
        loaderScript.FillSymbolsAndSectionPositions(symbols, sectionPositions);

        logFile << "Load remaining sections" << std::endl;
        FillRemainingSections();

        logFile << "Check output " << std::endl;
        FixRelocations();

        logFile << "Generating binary output" << std::endl;
        generateOutput();

        logFile << "Write output " << std::endl;
        WriteOutputFile(outputFile);
    }

    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        logFile << "ERROR: " << e.what() << std::endl;
    }

}

void Linker::AddSymbol(Symbol &sym) {
    auto symbolOld = symbols.find(sym.name);

    if (symbolOld == symbols.end()) {
        symbols.insert({sym.name, sym});
    } else if (sym.defined) {
        if (symbolOld->second.defined) {
            throw runtime_error("Defined symbol twice ! " + symbolOld->second.name);
        } else {
            symbolOld->second.defined = true;
            symbolOld->second.offset = sym.offset;
            symbolOld->second.sectionName = sym.sectionName;
            symbolOld->second.scope = ScopeType::GLOBAL;
            symbolOld->second.symbolType = sym.symbolType;
        }
    }
}

int Linker::AddSection(Symbol &sym, Section &section) {
    auto symbolOld = symbols.find(sym.name);

    int offset;

    if (symbolOld == symbols.end()) {
        symbols.insert({sym.name, sym});
        offset = 0;
    } else {
        offset = symbolOld->second.size;

        symbolOld->second.size += sym.size;

        sections.find(sym.name)->second += section;
    }

    return offset;
}

void Linker::LoadFile(std::string inputFile) {


    ObjectFile objectFile;

    objectFile.LoadFromFile(inputFile);

    unordered_map<string, int> sectionOffsets;

//    for (auto &section: objectFile.sections)
//    {
//        sections.insert({section.first, section.second});
//    }

    for (auto &symbol: objectFile.symbols) {
        if (symbol.second.symbolType == TokenType::SECTION) {
            //TODO: kobasica
            auto section = objectFile.sections.find(symbol.second.name);
            sections.insert({section->first, section->second});

            logFile << "Adding section symbol " << symbol.second.name << std::endl;
            sectionOffsets[symbol.second.name] = AddSection(symbol.second,
                                                            objectFile.sections.find(symbol.second.name)->second);
        }
    }

    for (auto &symbol: objectFile.symbols) {
        if (symbol.second.symbolType == TokenType::LABEL) {
            logFile << "Adding label symbol " << symbol.second.name << std::endl;
            symbol.second.offset += sectionOffsets[symbol.second.sectionName];
            AddSymbol(symbol.second);
        }
    }

    for (auto &relocation: objectFile.relocations) {
        //TODO: error ?
        relocation.offset += sectionOffsets[relocation.section];
        relocations.push_back(relocation);
    }

}

void Linker::FillRemainingSections() {
    vector <string> remainingSections;

    for (auto &symbol: symbols) {
        if (symbol.second.symbolType == TokenType::SECTION &&
            sectionPositions.find(symbol.second.name) == sectionPositions.end()) {
            remainingSections.push_back(symbol.second.name);
        }
    }

    sort(remainingSections.begin(), remainingSections.end());

    locationCounter = 0;
    for (auto &section: sectionPositions) {
        if (section.second > locationCounter) {
            //TODO: error ?
            locationCounter = section.second + sections.find(section.first)->second.size;
        }
    }

    for (auto &sectionName: remainingSections) {
        auto section = sections.find(sectionName)->second;
        int size = section.size;

        sectionPositions.insert({sectionName, locationCounter});

        locationCounter += size;
    }
}

u_int32_t Linker::GetSymbolVal(std::string symbolName) {
    u_int32_t ret = 0;

    auto symbol = symbols.find(symbolName);
    if (symbol == symbols.end()) {
        throw runtime_error("Symbol not found " + symbolName);
    }

    if (symbol->second.symbolType == TokenType::LABEL || symbol->second.symbolType == TokenType::OPERAND_DEC) {
        ret += symbol->second.offset;
    }

    if (symbol->second.symbolType == TokenType::LABEL || symbol->second.symbolType == TokenType::SECTION) {
        string sectionName = symbol->second.sectionName;
        auto sectionPos = sectionPositions.find(sectionName);
        if (sectionPos == sectionPositions.end()) {
            throw runtime_error("rand err? " + sectionName);
        }

        ret += sectionPos->second;
    }

    return ret;
}

void Linker::FixRelocations() {
    vector <Relocation> newRelocations;
    for (auto &rel: relocations) {
        auto symbol = symbols.find(rel.symbolName);

        if (symbol == symbols.end()) {
            throw runtime_error("rand err");
        }

        if (symbol->second.defined) {
            u_int32_t symbolVal = GetSymbolVal(symbol->second.name);

            auto section = sections.find(rel.section);

            if (section == sections.end()) {
                throw runtime_error("random errr");
            }

            if (section->second.size <= rel.offset) {
                cout << section->second.size << " " << rel.offset << std::endl;
                throw runtime_error("randddddom err");
            }

            if (rel.relocationType == LONG) {
                section->second.Write(&symbolVal, rel.offset, 4);
            } else if (rel.relocationType == LDCRELOC) {
                u_int16_t high = symbolVal >> 16;
                u_int16_t low = symbolVal;
                section->second.Write(&high, rel.offset + 2, 2);
                section->second.Write(&low, rel.offset + 6, 2);
            }
        } else {
            newRelocations.push_back(rel);
        }
    }

    relocations = newRelocations;
}

void Linker::WriteOutputFile(std::ofstream &outputFile) {
    for (auto &symbol: symbols)
        logFile << symbol.second;

    for (int i = 0; i < relocations.size(); i++)
        logFile << (relocations[i]);

    for (auto &section: sections)
        logFile << section.second;

    outputFile << "%SYMBOLS SECTION%" << std::endl;

    outputFile << left << "  " <<
               std::setw(15) << "Symbol" <<
               std::setw(15) << "SymbolName" <<
               std::setw(15) << "Defined" <<
               std::setw(15) << "SectionName" <<
               std::setw(15) << "Offset" <<
               std::setw(15) << "Type" <<
               std::setw(15) << "Size" <<
               std::setw(15) << "SymbolType" <<
               std::endl;

    for (auto &symbol: symbols)
        outputFile << symbol.second.Serialize();

    outputFile << "%END%" << std::endl;

    outputFile << std::endl;

    outputFile << "%RELOCATIONS SECTION%" << std::endl;

    outputFile << std::left << "  " <<
               std::setw(15) << "Relocation" <<
               std::setw(15) << "SymbolName" <<
               std::setw(15) << "SectionName" <<
               std::setw(15) << "Offset" <<
               std::setw(15) << "Type" <<
               std::endl;

    for (auto &rel: relocations) {
        outputFile << rel.Serialize();
    }

    outputFile << "%END%" << std::endl;

    outputFile << std::endl << std::left;

    outputFile << "%SECTIONS SECTION%" << std::endl;

    for (auto &section: sections) {
        outputFile << section.second.Serialize();
    }

    outputFile << "%END%" << std::endl << std::endl;

    for (auto &section: sectionPositions) {
        outputFile << section.first << " - " << section.second << std::endl;
    }

    outputFile << std::endl;

    if (symbols.find("main") == symbols.end()) {
        outputFile << "***** ERROR: Main not defined." << std::endl;
        return;
    }
    if (!outputSection) {
        outputFile << "***** ERROR: Not all realocations fixed." << std::endl;
        return;
    }

    outputFile << "%OUTPUT SECTION%" << std::endl;

    Symbol mainSym = symbols.find("main")->second;
    outputFile << "Main: " << (mainSym.offset + sectionPositions[mainSym.sectionName]) << std::endl;

    outputFile << outputSection->Serialize();

    outputFile << "%END%" << std::endl;

}

void Linker::generateOutput() {
    if (!relocations.empty()) {
        logFile << "not all relocations fixed" << std::endl;
        return;
    }

    outputSection = new Section("output", locationCounter);

    //memory = new u_int8_t[locationCounter];

    outputSection->WriteZeros(0, locationCounter);

    for (auto &sectionPosition: sectionPositions) {
        std::string sectionName = sectionPosition.first;
        int sectionStart = sectionPosition.second;

        auto section = sections.find(sectionName);
        //TODO: error ?

        outputSection->Write(section->second.memory, sectionStart, section->second.size);
//        memcpy(memory + sectionStart,section->second.memory, section->second.size);
    }
}