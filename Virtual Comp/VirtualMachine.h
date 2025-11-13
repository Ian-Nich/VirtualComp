#pragma once             // include this file once per compilation unit
#include <vector>
#include <string>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <functional>
#include <unordered_map>

class VirtualMachine {
private:
    // --- machine state ---
    std::vector<std::string> instructions;
    std::vector<int> stack;
    int counter = 0;
    int pc = 0;
    std::vector<int> memory;
    std::vector<int> registers = std::vector<int>(8, 0);  // R0–R7
    bool flag_eq = false, flag_gt = false, flag_lt = false;

    struct Instruction {
        uint8_t opcode;
        uint8_t a, b, c;
    };

    std::vector<Instruction> bytecode;
    std::unordered_map<uint8_t, std::function<void(const Instruction&)>> dispatch;

    // stepper/trace
    bool trace = false;
    bool explain = false;
    std::unordered_set<int> breakpoints; // 1-based PCs

public:
    VirtualMachine();

    // text-mode (legacy) loader/runner — still declared because .cpp has them
    void loadProgram(const std::string& filename);
    void run();

    // bytecode path
    void loadBytecode(const std::string& filename);
    void initializeDispatchTable();
    void runBytecode();             // fast path
    void runBytecodeStep();         // REPL/stepper
    void disassemble() const;       // dump bytecode as text

    // stepper controls
    void setTrace(bool on)   { trace = on; }
    void setExplain(bool on) { explain = on; }
    void addBreakpoint(int one_based_pc);
    void setBreakpoints(const std::vector<int>& bps);
    void printHelp() const;

private:
    // text-mode executor
    void execute(const std::string& instrLine);

    // helpers
    bool isValidAddr(int addr) const;

    int getRegisterIndex(const std::string& token) {
        if (token.size() != 2 || token[0] != 'R') return -1;
        return token[1] - '0';  // R0–R7
    }

    void printState() const;                          // regs/stack/mem/flags
    void printInstruction(const Instruction&) const;  // pretty instruction
    const char* opcodeName(uint8_t op) const;         // mnemonic
    void dumpRegs() const;
    void dumpStack() const;
    void dumpMem(int start, int len) const;
};



/*
pragma once replaes this! (but tis less reliable)

"This works by defining a macro when the file is first included. 
If the file is included again, the macro prevents the content from being reprocessed.
"

#ifndef R.O.B
#define R.O.B
#endif

pragma can fail with symbolic links or duplicate paths
#pragma once is preferred unless you’re dealing with unusual or old compilers.
zzzzz




uint8_t is defined in <cstdint> (C++’s version of <stdint.h>)

Without this header, your code doesn’t know what fixed-width integer types are

If you're writing portable C++ (like on Linux, Windows, or embedded), <cstdint> is the correct and modern header to include.
*/

