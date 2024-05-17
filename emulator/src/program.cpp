#include <iostream>
#include <cstring>
#include "program.h"
#include "emulator.h"

Program::Program() = default;

void Program::loadSection(std::istream &inputFile) {

    std::string line;
    std::string sectionCumulate;

    bool parsedOutputSection = false;
    bool outputSection = false;
    bool mainOutputPassed = false;

    while (getline(inputFile, line)) {
        if (!outputSection) {
            if (line == "%OUTPUT SECTION%") {
                outputSection = true;
                parsedOutputSection = true;
            }
            continue;
        }
        if (line == "%END%") {
            outputSection = false;
            continue;
        }

        if (!mainOutputPassed) {
            mainOutputPassed = true;
            std::stringstream ss;
            ss << line;
            std::string token;
            ss >> token;
            ss >> startPoint;
            continue;
        }

        if (line == ".end") {
            programSection = Section::Deserialize(sectionCumulate);
            sectionCumulate.clear();
            Emulator::logFile << "Copied code to " << CODE_START << " " << (int) memory[64] << std::endl;
            memcpy(memory + CODE_START, programSection.memory, programSection.size);
            startPoint += CODE_START;
        } else
            sectionCumulate += line + "\n";
    }

    if (!parsedOutputSection)
        throw std::runtime_error("No code to emulate !");
}

void Program::loadDefaultIVT(std::ifstream &inputFile) {

    for (int i = 0; i < 16; ++i) {
        u_int32_t ivtPoint = IVT_START + i * IVT_SIZE - CODE_START;
        //Emulator::logFile << "Copied code to " << 4*i << " " << (int)memory[64] << std::endl;
        memcpy(memory + 4 * i, &ivtPoint, 4);

        //Emulator::logFile << "Copied code to " << ivtPoint << " " << (int)memory[64] << std::endl;
        memcpy(memory + ivtPoint, &IRET_CODE, 4);
    }

    if (inputFile.is_open()) {
        std::string line;
        std::string sectionCumulate;
        bool outputSection = false;
        Section currentSection("random", 0);

        while (getline(inputFile, line)) {
            if (!outputSection) {
                if (line == "%SECTIONS SECTION%")
                    outputSection = true;
                continue;
            }
            if (line == "%END%") {
                outputSection = false;
                continue;
            }

            if (line == ".end") {
                currentSection = Section::Deserialize(sectionCumulate);
                sectionCumulate.clear();

                if (currentSection.name == ".text.start") {
                    //Emulator::logFile << "Copied code to " << IVT_START <<" " << (int)memory[64] <<  endl;
                    memcpy(memory + IVT_START, currentSection.memory, currentSection.size);
                    Emulator::logFile << "yay, loaded start ivt" << std::endl;
                } else if (currentSection.name == ".text.timer") {
                    //Emulator::logFile << "Copied code to " << IVT_START + 1*IVT_SIZE <<" " << (int)memory[64] <<  endl;
                    memcpy(memory + IVT_START + 1 * IVT_SIZE, currentSection.memory, currentSection.size);
                    Emulator::logFile << "yay, loaded timer ivt" << std::endl;
                } else if (currentSection.name == ".text.illegal") {
                    //Emulator::logFile << "Copied code to " << IVT_START + 2*IVT_SIZE <<" " << (int)memory[64] <<  endl;
                    memcpy(memory + IVT_START + 2 * IVT_SIZE, currentSection.memory, currentSection.size);
                    Emulator::logFile << "yay, loaded illegal ivt" << std::endl;
                } else if (currentSection.name == ".text.keyboard") {
                    //Emulator::logFile << "Copied code to " << IVT_START + 3*IVT_SIZE <<" " << (int)memory[64] <<  endl;
                    memcpy(memory + IVT_START + 3 * IVT_SIZE, currentSection.memory, currentSection.size);
                    Emulator::logFile << "yay, loaded keyboard ivt" << std::endl;
                }

            } else
                sectionCumulate += line + "\n";
        }
    }
}

u_int32_t Program::signExt(u_int32_t val, size_t size) {
    val <<= 32 - size;
    int32_t temp = val;
    temp >>= 32 - size;
    val = temp;
    return val;
}

void Program::stackPush(u_int32_t val) {
    if (SP <= STACK_START)
        throw std::runtime_error("Stack overflow !");
    Emulator::logFile << "Stack push " << std::hex << val << std::endl;
    SP -= 4;
    memcpy(memory + SP, &val, 4);
}

void Program::init() {
    PC = startPoint;
    SP = STACK_SIZE + STACK_START;

    PSW.val = 0;
    PSW.T = 1;

    executionStart = std::chrono::system_clock::now();
    lastTimerExecution = executionStart;

    int iret1 = pthread_create(&keyboardThread, NULL, KeyboardThread, nullptr);

    if (iret1)
        throw std::runtime_error("Error - pthread_create() return code: " + iret1);

    //call first routine
    stackPush(LR);
    LR = PC;
    stackPush(PSW.val);
    PSW.I = 1;
    memcpy(&PC, memory, 4);
    PC += CODE_START;
}

void Program::readNext() {

    u_int32_t currentCode;

//    if (PC >= programSection.size + 4)
//        throw runtime_error("Out of scope ! Please use halt at the end of program.");

    memcpy(&currentCode, memory + PC, 4);
    currInstr = Instruction::Deserialize(currentCode);

    PC += 4;
}

void Program::setMemory(u_int32_t val, u_int32_t ind) {
    if (ind >= MEMORY_SIZE)
        throw std::runtime_error("Index for memory out of bounds !");

    Emulator::logFile << "Set memory " << ind << " - " << val << std::endl;

    memcpy(memory + ind, &val, 4);
}

void Program::executeCurrent() {

    Emulator::logFile << "Registers: ";
    for (int i = 0; i < 16; i++)
        Emulator::logFile << " " << std::hex << registers[i];

    Emulator::logFile << std::endl;
    Emulator::logFile << "PC: " << std::hex << PC << " ";
    Emulator::logFile << "LR: " << std::hex << LR << " ";
    Emulator::logFile << "SP: " << std::hex << SP << " ";
    Emulator::logFile << "PSW: " << std::hex << PSW.val << std::endl;

    Emulator::logFile << "Executing: " << currInstr.instructionSymbol << " "
                      << currInstr.instructionCondition << " " << currInstr.setFlags << " "
                      << currInstr.instrCode.binaryCode << std::endl;

    if (conditionTesters[currInstr.instructionCondition]())
        instructionExecutors[currInstr.instructionSymbol]();
    else
        Emulator::logFile << "Skipping instruction" << std::endl;


    Emulator::logFile << "Registers: ";
    for (int i = 0; i < 16; i++)
        Emulator::logFile << " " << std::hex << registers[i];

    Emulator::logFile << std::endl;
    Emulator::logFile << "PC: " << std::hex << PC << " ";
    Emulator::logFile << "LR: " << std::hex << LR << " ";
    Emulator::logFile << "SP: " << std::hex << SP << " ";
    Emulator::logFile << "PSW: " << std::hex << PSW.val << std::endl;

    handleInterrupts();
    Emulator::logFile << std::endl;

}

bool Program::isEnd() {
    return (currInstr.instrCode.instruction.instr == InstructionCodes::HALT);
}

void Program::handleInterrupts() {

    std::chrono::time_point<std::chrono::system_clock> timeNow = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = timeNow - lastTimerExecution;
    //TODO: hardcoded
    //Emulator::logFile << dec << "elapsed: " << elapsed.count() << std::endl;
    if (elapsed.count() >= 1.0) {
        lastTimerExecution = timeNow;
        timerInterrupt();
    }

    if (keyboardInterrupt)
        KeyboardInterrupt();

    if (memory[OUTPUT_POS] != 0) {
        char temp = memory[OUTPUT_POS];
        memory[OUTPUT_POS] = 0;

        std::cout << temp;
        std::cout.flush();
        //cout << "Print char ! : " << temp << std::endl;
        Emulator::logFile << "Print char: " << temp << std::endl;
    }


}

void Program::KeyboardInterrupt() {
    if (PSW.I == 1) {
        Emulator::logFile << "Masked interrupts (keyboard)" << std::endl;
        return;
    }

    Emulator::logFile << "Keyboard interrupt !" << std::endl;

    //copy the contents of buf

    u_int32_t temp = keyboardBuf;
    keyboardInterrupt = false;

    memcpy(memory + KEYBOARD_POS, &temp, 4);
    memcpy(memory + KEYBOARD_STATUS_POS, &KEYBOARD_STATUS_MASK, 4);

    stackPush(LR);
    LR = PC;
    stackPush(PSW.val);
    PSW.I = 1;

    memcpy(&PC, memory + 12, 4);

    PC += CODE_START;
}

void Program::timerInterrupt() {
    if (PSW.I == 1) {
        Emulator::logFile << "Masked interrupts (timer)" << std::endl;
        return;
    }
    if (PSW.T == 0) {
        Emulator::logFile << "Masked timer interrupt" << std::endl;
        return;
    }

    Emulator::logFile << "Timer interrupt !" << std::endl;

    stackPush(LR);
    LR = PC;
    stackPush(PSW.val);
    PSW.I = 1;
    memcpy(&PC, memory + 4, 4);
    PC += CODE_START;
}

void Program::setRegister(u_int32_t val, u_int32_t ind) {
    switch (ind) {
        case 16:
            PC = val;
            break;
        case 17:
            LR = val;
            break;
        case 18:
            SP = val;
            break;
        case 19:
            PSW.val = val;
            break;
        default:
            registers[ind] = val;
    }
}

u_int32_t Program::getMemory(u_int32_t ind) {
    if (ind >= MEMORY_SIZE)
        throw std::runtime_error("Index for memory out of bounds !");

    u_int32_t res;

    memcpy(&res, memory + ind, 4);

    Emulator::logFile << "Fetched memory from " << ind << " - " << res << std::endl;

    return res;
}

u_int32_t Program::getRegister(u_int32_t ind) {
    switch (ind) {
        case 16:
            return PC;
        case 17:
            return LR;
        case 18:
            return SP;
        case 19:
            return PSW.val;
        default:
            return registers[ind];
    }
}

void *Program::KeyboardThread(void *ptr) {
    while (true) {
        while (keyboardInterrupt);

        //cout << "Passed barrier" << std::endl;
        Emulator::logFile << "Passed barrier" << std::endl;

        u_int8_t temp;

        std::cin >> std::noskipws >> temp;

        //cout << "Read char " << temp << std::endl;
        Emulator::logFile << "Read char " << temp << std::endl;

        keyboardBuf = temp;

        keyboardInterrupt = true;
    }
}


u_int32_t Program::stackPop() {
    if (SP >= STACK_START + STACK_SIZE) {
        throw std::runtime_error("Stack underflow !");
    }

    u_int32_t ret;
    std::memcpy(&ret, memory + SP, 4);

    SP += 4;

    Emulator::logFile << "Stack pop " << std::hex << ret << std::endl;

    return ret;
}

