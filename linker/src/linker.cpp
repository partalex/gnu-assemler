#include "../include/linker.h"
#include "../include/object_file.h"

#include <regex>
#include <iostream>
#include <iomanip>
#include <fstream>

std::ofstream Linker::_logFile("linker/log");

std::regex sectionRegex("^.(text|data|bss)(.[a-zA-Z_][a-zA-Z0-9]*)?$");

void Linker::link(std::ifstream &loaderScriptFile, std::vector<std::string> &inputFiles, std::ofstream &outputFile) {
    try {
        _logFile << "Load script file" << "\n";
        _loaderScript.loadFromFile(loaderScriptFile);
        for (auto &inputFile: inputFiles) {
            _logFile << "Linking file " << "\n";
            loadFile(inputFile);
        }
        _logFile << "Load symbols from script" << "\n";
        _loaderScript.fillSymbolsAndSectPos(_symbols, _sectionPositions);
        _logFile << "Load remaining sections" << "\n";
        fillRemainingSections();
        _logFile << "Check output " << "\n";
        fixRelocations();
        _logFile << "Generating binary output" << "\n";
        generateOutput();
        _logFile << "Write output " << "\n";
        writeOutputFile(outputFile);
    }
    catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        _logFile << "ERROR: " << e.what() << "\n";
    }
}

void Linker::addSymbol(Symbol &sym) {
    auto symbolOld = _symbols.find(sym._name);
    if (symbolOld == _symbols.end())
        _symbols.insert({sym._name, sym});
    else if (sym._defined) {
        if (symbolOld->second._defined)
            throw std::runtime_error("Defined symbol twice ! " + symbolOld->second._name);
        else {
            symbolOld->second._defined = true;
            symbolOld->second._offset = sym._offset;
            symbolOld->second._sectionName = sym._sectionName;
            symbolOld->second._scope = SCOPE::GLOBAL;
            symbolOld->second._symbolType = sym._symbolType;
        }
    }
}

int Linker::addSection(Symbol &sym, Section &section) {
    auto symbolOld = _symbols.find(sym._name);
    int offset;
    if (symbolOld == _symbols.end()) {
        _symbols.insert({sym._name, sym});
        offset = 0;
    } else {
        offset = symbolOld->second._size;
        symbolOld->second._size += sym._size;
        _sections.find(sym._name)->second += section;
    }
    return offset;
}

void Linker::loadFile(const std::string &inputFile) {
    ObjectFile objectFile;
    objectFile.loadFromFile(inputFile);
    std::unordered_map<std::string, int> sectionOffsets;
    for (auto &symbol: objectFile._symbols)
        if (symbol.second._symbolType == SYMBOL::SECTION) {
            auto section = objectFile._sections.find(symbol.second._name);
            _sections.insert({section->first, section->second});
            _logFile << "Adding section symbol " << symbol.second._name << "\n";
            sectionOffsets[symbol.second._name] = addSection(symbol.second,
                                                             objectFile._sections.find(symbol.second._name)->second);
        }
    for (auto &symbol: objectFile._symbols)
        if (symbol.second._symbolType == SYMBOL::LABEL) {
            _logFile << "Adding label symbol " << symbol.second._name << "\n";
            symbol.second._offset += sectionOffsets[symbol.second._sectionName];
            addSymbol(symbol.second);
        }
    for (auto &relocation: objectFile._relocations) {
        relocation._offset += sectionOffsets[relocation._section];
        _relocations.push_back(relocation);
    }
}

void Linker::fillRemainingSections() {
    std::vector<std::string> remainingSections;
    for (auto &symbol: _symbols)
        if (symbol.second._symbolType == SYMBOL::SECTION &&
            _sectionPositions.find(symbol.second._name) == _sectionPositions.end()) {
            remainingSections.push_back(symbol.second._name);
        }
    sort(remainingSections.begin(), remainingSections.end());
    _locationCounter = 0;
    for (auto &section: _sectionPositions)
        if (section.second > _locationCounter)
            _locationCounter = section.second + _sections.find(section.first)->second._size;
    for (auto &sectionName: remainingSections) {
        auto section = _sections.find(sectionName)->second;
        int size = section._size;
        _sectionPositions.insert({sectionName, _locationCounter});
        _locationCounter += size;
    }
}

uint32_t Linker::getSymbolVal(const std::string &symbolName) {
    uint32_t ret = 0;
    auto symbol = _symbols.find(symbolName);
    if (symbol == _symbols.end())
        throw std::runtime_error("Symbol not found " + symbolName);
    if (symbol->second._symbolType == SYMBOL::LABEL || symbol->second._symbolType == SYMBOL::OPERAND_DEC)
        ret += symbol->second._offset;
    if (symbol->second._symbolType == SYMBOL::LABEL || symbol->second._symbolType == SYMBOL::SECTION) {
        std::string sectionName = symbol->second._sectionName;
        auto sectionPos = _sectionPositions.find(sectionName);
        if (sectionPos == _sectionPositions.end())
            throw std::runtime_error("rand err? " + sectionName);
        ret += sectionPos->second;
    }
    return ret;
}

void Linker::fixRelocations() {
    std::vector<Relocation> newRelocations;
    for (auto &rel: _relocations) {
        auto symbol = _symbols.find(rel._symbolName);
        if (symbol == _symbols.end())
            throw std::runtime_error("rand err");
        if (symbol->second._defined) {
            uint32_t symbolVal = getSymbolVal(symbol->second._name);
            auto section = _sections.find(rel._section);
            if (section == _sections.end())
                throw std::runtime_error("random errr");
            if (section->second._size <= rel._offset) {
                std::cout << section->second._size << " " << rel._offset << "\n";
                throw std::runtime_error("randddddom err");
            }
            if (rel._relocationType == RELOCATION::R_386_32)
                section->second.write(&symbolVal, rel._offset, 4);
            else if (rel._relocationType == RELOCATION::R_386_PC32) {
                u_int16_t high = symbolVal >> 16;
                u_int16_t low = symbolVal;
                section->second.write(&high, rel._offset + 2, 2);
                section->second.write(&low, rel._offset + 6, 2);
            }
        } else
            newRelocations.push_back(rel);
    }
    _relocations = newRelocations;
}

void Linker::writeOutputFile(std::ofstream &outputFile) {
    for (auto &symbol: _symbols)
        _logFile << symbol.second;
    for (auto &relocation: _relocations)
        _logFile << relocation;
    for (auto &section: _sections)
        _logFile << section.second;
    outputFile << "%SYMBOLS SECTION%" << "\n";
    outputFile << std::left << "  " <<
               std::setw(15) << "Symbol" <<
               std::setw(15) << "SymbolName" <<
               std::setw(15) << "Defined" <<
               std::setw(15) << "SectionName" <<
               std::setw(15) << "Offset" <<
               std::setw(15) << "Type" <<
               std::setw(15) << "Size" <<
               std::setw(15) << "SymbolType" << "\n";
    for (auto &symbol: _symbols)
        outputFile << symbol.second.serialize();
    outputFile << "%END%" << "\n";
    outputFile << "\n";
    outputFile << "%RELOCATIONS SECTION%" << "\n";
    outputFile << std::left << "  " <<
               std::setw(15) << "Relocation" <<
               std::setw(15) << "SymbolName" <<
               std::setw(15) << "SectionName" <<
               std::setw(15) << "Offset" <<
               std::setw(15) << "Type" << "\n";
    for (auto &rel: _relocations)
        outputFile << rel.serialize();
    outputFile << "%END%" << "\n";
    outputFile << "\n" << std::left;
    outputFile << "%SECTIONS SECTION%" << "\n";
    for (auto &section: _sections)
        outputFile << section.second.serialize();
    outputFile << "%END%" << "\n" << "\n";
    for (auto &section: _sectionPositions)
        outputFile << section.first << " - " << section.second << "\n";
    outputFile << "\n";
    if (_symbols.find("main") == _symbols.end()) {
        outputFile << "***** ERROR: Main not defined." << "\n";
        return;
    }
    if (!outputSection) {
        outputFile << "***** ERROR: Not all realocations fixed." << "\n";
        return;
    }
    outputFile << "%OUTPUT SECTION%" << "\n";
    Symbol mainSym = _symbols.find("main")->second;
    outputFile << "Main: " << (mainSym._offset + _sectionPositions[mainSym._sectionName]) << "\n";
    outputFile << outputSection->serialize();
    outputFile << "%END%" << "\n";
}

void Linker::generateOutput() {
    if (!_relocations.empty()) {
        _logFile << "not all relocations fixed" << "\n";
        return;
    }
    outputSection = new Section("output", _locationCounter);
    outputSection->WriteZeros(0, _locationCounter);
    for (auto &sectionPosition: _sectionPositions) {
        std::string sectionName = sectionPosition.first;
        int sectionStart = sectionPosition.second;
        auto section = _sections.find(sectionName);
        outputSection->Write(section->second._memory, sectionStart, section->second._size);
    }
}