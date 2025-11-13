#include "VirtualMachine.h"
#include <stack>
#include <vector>
#include <iostream>

enum Opcodes {
    OP_PUSH     = 0x01,
    OP_MOV      = 0x02,
    OP_ADDR     = 0x03,
    OP_SUBR     = 0x04,
    OP_LOADR    = 0x05,
    OP_STORER   = 0x06,
    OP_PRINT    = 0x07,
    OP_PRINTR   = 0x08,
    OP_CMP      = 0x09,
    OP_JEQ      = 0x0A,
    OP_JNE      = 0x0B,
    OP_JGT      = 0x0C,
    OP_JLT      = 0x0D,
    OP_LOADM    = 0x0E,
    OP_STOREM   = 0x0F,
    OP_LOADMR   = 0x10,
    OP_STOREMR  = 0x11,
    OP_DECR     = 0x12,
    OP_CPRINT   = 0x13,
    OP_CEASE    = 0x14
};

const char* VirtualMachine::opcodeName(uint8_t op) const {
    switch (op) {
        case OP_PUSH: return "PUSH";
        case OP_MOV: return "MOV";
        case OP_ADDR: return "ADDR";
        case OP_SUBR: return "SUBR";
        case OP_LOADR: return "LOADR";
        case OP_STORER: return "STORER";
        case OP_PRINT: return "PRINT";
        case OP_PRINTR: return "PRINTR";
        case OP_CMP: return "CMP";
        case OP_JEQ: return "JEQ";
        case OP_JNE: return "JNE";
        case OP_JGT: return "JGT";
        case OP_JLT: return "JLT";
        case OP_LOADM: return "LOADM";
        case OP_STOREM: return "STOREM";
        case OP_LOADMR: return "LOADMR";
        case OP_STOREMR: return "STOREMR";
        case OP_DECR: return "DECR";
        case OP_CPRINT: return "CPRINT";
        case OP_CEASE: return "CEASE";
        default: return "???";
    }
}

void VirtualMachine::printInstruction(const Instruction& ins) const {
    // show 1-based PC to match your assembler/jump semantics
    std::cout << "PC " << (pc + 1) << ": " << opcodeName(ins.opcode)
              << " " << int(ins.a) << " " << int(ins.b) << " " << int(ins.c) << "\n";
    if (explain) {
        // a few human-friendly hints (expand as you like)
        switch (ins.opcode) {
            case OP_MOV:    std::cout << "  -> R" << int(ins.a) << " = " << int(ins.b) << "\n"; break;
            case OP_ADDR:   std::cout << "  -> R" << int(ins.a) << " = R" << int(ins.b) << " + R" << int(ins.c) << "\n"; break;
            case OP_LOADMR: std::cout << "  -> R" << int(ins.a) << " = MEM[" << int(ins.b) << "]\n"; break;
            case OP_STOREMR:std::cout << "  -> MEM[" << int(ins.a) << "] = R" << int(ins.b) << "\n"; break;
            case OP_CMP:    std::cout << "  -> set flags by comparing "
                                      << ((ins.a==0xFF) ? "COUNTER" : ("R"+std::to_string(ins.a)))
                                      << " vs "
                                      << ((ins.b==0xFF) ? "COUNTER" : ("R"+std::to_string(ins.b))) << "\n"; break;
            case OP_JEQ:    std::cout << "  -> jump if EQ to line " << int(ins.a) << "\n"; break;
            case OP_JGT:    std::cout << "  -> jump if GT to line " << int(ins.a) << "\n"; break;
            case OP_JLT:    std::cout << "  -> jump if LT to line " << int(ins.a) << "\n"; break;
            default: break;
        }
    }
}


void VirtualMachine::printState() const {
    std::cout << "REGS: ";
    for (size_t i = 0; i < registers.size(); ++i) {
        std::cout << "R" << i << "=" << registers[i] << (i+1<registers.size()?" ":"");
    }
    std::cout << "   COUNTER=" << counter << "   FLAGS[EQ=" << flag_eq
              << " GT=" << flag_gt << " LT=" << flag_lt << "]\n";

    std::cout << "STACK: [";
    for (size_t i = 0; i < stack.size(); ++i) {
        std::cout << stack[i] << (i+1<stack.size()?", ":"");
    }
    std::cout << "]\n";

    // show a small memory window (non-zero cells) for clarity
    bool any=false;
    for (size_t i = 0; i < memory.size(); ++i) {
        if (memory[i] != 0) { any=true; break; }
    }
    if (any) {
        std::cout << "MEM (non-zero): ";
        bool first=true;
        for (size_t i = 0; i < memory.size(); ++i) {
            if (memory[i] != 0) {
                if (!first) std::cout << " | ";
                std::cout << "[" << i << "]=" << memory[i];
                first=false;
            }
        }
        std::cout << "\n";
    }
}


void VirtualMachine::runBytecodeStep() {
    auto at_breakpoint = [this]() {
        int one_based_pc = pc + 1;
        return breakpoints.count(one_based_pc) > 0;
    };

    auto exec_one = [this]() {
        const Instruction& instr = bytecode[pc];
        if (trace) printInstruction(instr);

        auto it = dispatch.find(instr.opcode);
        if (it != dispatch.end()) it->second(instr);
        else std::cerr << "Unknown opcode: 0x" << std::hex << int(instr.opcode) << std::dec << "\n";
    };

    auto disasm_one = [this](int i) {
        const Instruction& ins = bytecode[i];
        std::cout << (i+1) << ": " << opcodeName(ins.opcode)
                  << " " << int(ins.a) << " " << int(ins.b) << " " << int(ins.c) << "\n";
    };

    std::cout << "Stepper started. Type 'help' for commands.\n";

    for (pc = 0; pc < (int)bytecode.size(); /* pc advanced in loop */) {
        // Pause if at breakpoint or at the beginning
        if (at_breakpoint() || pc == 0) {
            // Show current instruction and state
            printInstruction(bytecode[pc]);
            printState();
            // REPL
            for (;;) {
                std::cout << "(vm) ";
                std::string cmd; 
                if (!std::getline(std::cin, cmd)) return;
                std::istringstream iss(cmd);
                std::string t; iss >> t;
                if (t == "" ) continue;

                if (t == "help") { printHelp(); continue; }
                if (t == "regs") { dumpRegs(); continue; }
                if (t == "stack"){ dumpStack(); continue; }
                if (t == "mem")  { int s,n; if (iss>>s>>n) dumpMem(s,n); else std::cout<<"usage: mem <start> <n>\n"; continue; }
                if (t == "bp") {
                    std::string sub; iss>>sub;
                    if (sub=="add"){ int n; if (iss>>n){ addBreakpoint(n); std::cout<<"added bp at "<<n<<"\n"; } else std::cout<<"usage: bp add <n>\n"; }
                    else if (sub=="del"){ int n; if (iss>>n){ breakpoints.erase(n); std::cout<<"removed bp "<<n<<"\n"; } else std::cout<<"usage: bp del <n>\n"; }
                    else if (sub=="list"){ if (breakpoints.empty()) std::cout<<"(none)\n"; else { for(int b:breakpoints) std::cout<<b<<" "; std::cout<<"\n"; } }
                    else if (sub=="clear"){ breakpoints.clear(); std::cout<<"All breakpoints cleared.\n"; }
                    else std::cout<<"usage: bp [add|del|list|clear] ...\n";
                    continue;
                }
                if (t == "trace"){ std::string on; iss>>on; if(on=="on")trace=true; else if(on=="off")trace=false; else std::cout<<"usage: trace on|off\n"; continue; }
                if (t == "explain"){ std::string on; iss>>on; if(on=="on")explain=true; else if(on=="off")explain=false; else std::cout<<"usage: explain on|off\n"; continue; }
                if (t == "disasm"){ disassemble(); continue; }

                if (t == "step" || t == "s") {
                    exec_one();
                    // advance pc unless handler changed it (jumps already set pc)
                    ++pc;
                    break; // leave REPL to re-check bp and show next state
                }
                if (t == "cont" || t == "c") {
                    // run until breakpoint or end
                    while (pc < (int)bytecode.size() && !at_breakpoint()) {
                        exec_one();
                        ++pc;
                    }
                    break; // will re-show state at next loop
                }
                if (t == "quit" || t == "q") { return; }

                std::cout << "Unknown command. Type 'help'.\n";
            }
        } else {
            // Not at breakpoint: single-step automatically
            exec_one();
            ++pc;
        }
    }
}


void VirtualMachine::disassemble() const {
    for (size_t i = 0; i < bytecode.size(); ++i) {
        const Instruction& ins = bytecode[i];
        // print 1-based address to match your assembler labels/jumps
        std::cout << (i + 1) << ": " << opcodeName(ins.opcode)
                  << " " << int(ins.a) << " " << int(ins.b) << " " << int(ins.c) << "\n";
    }
}


void VirtualMachine::addBreakpoint(int one_based_pc) {
    if (one_based_pc > 0) breakpoints.insert(one_based_pc);
}
void VirtualMachine::setBreakpoints(const std::vector<int>& bps) {
    breakpoints.clear();
    for (int b : bps) if (b > 0) breakpoints.insert(b);
}

void VirtualMachine::dumpRegs() const {
    std::cout << "REGS:";
    for (size_t i = 0; i < registers.size(); ++i)
        std::cout << " R" << i << "=" << registers[i];
    std::cout << "  COUNTER=" << counter
              << "  FLAGS[EQ=" << flag_eq << " GT=" << flag_gt << " LT=" << flag_lt << "]\n";
}
void VirtualMachine::dumpStack() const {
    std::cout << "STACK: [";
    for (size_t i = 0; i < stack.size(); ++i)
        std::cout << stack[i] << (i+1<stack.size()?", ":"");
    std::cout << "]\n";
}
void VirtualMachine::dumpMem(int start, int len) const {
    if (start < 0) start = 0;
    int end = std::min<int>(start + len, (int)memory.size());
    for (int i = start; i < end; ++i) {
        std::cout << "[" << i << "]=" << memory[i] << ((i+1<end)?"  ":"\n");
    }
}


void VirtualMachine::printHelp() const {
    std::cout <<
    "Commands:\n"
    "  step / s         Execute next instruction\n"
    "  cont / c         Continue running until CEASE or breakpoint\n"
    "  regs             Show registers, counter, flags\n"
    "  stack            Show stack\n"
    "  mem <start> <n>  Show n memory cells starting at start\n"
    "  bp add <n>       Add breakpoint at line n (1-based)\n"
    "  bp del <n>       Remove breakpoint\n"
    "  bp list          List breakpoints\n"
    "  trace on|off     Toggle raw instruction trace\n"
    "  explain on|off   Toggle human explanations\n"
    "  disasm           Disassemble loaded bytecode\n"
    "  help             Show this help\n"
    "  quit / q         Exit stepper\n";
    }


VirtualMachine::VirtualMachine() : memory(256, 0), registers(8, 0) {
    initializeDispatchTable();
}                              // initialize 256 memory cells with 0



void VirtualMachine::loadBytecode(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open bytecode file\n";
        return;
    }

    Instruction instr;
    while (file.read(reinterpret_cast<char*>(&instr), sizeof(instr))) {
        bytecode.push_back(instr);
    }
}

void VirtualMachine::runBytecode() {
    for (pc = 0; pc < bytecode.size(); ++pc) {
        const Instruction& instr = bytecode[pc];

        auto it = dispatch.find(instr.opcode);
        if (it != dispatch.end()) {
            it->second(instr);
        } else {
            std::cerr << "Unknown opcode: 0x" << std::hex << int(instr.opcode) << std::dec << std::endl;
        }
    }
}



void VirtualMachine::initializeDispatchTable() {
    dispatch[OP_PUSH] = [this](const Instruction& instr) {
        stack.push_back(instr.a);
    };

    dispatch[OP_MOV] = [this](const Instruction& instr) {
        if (instr.a < registers.size()) {
            registers[instr.a] = instr.b;
        }
    };

    dispatch[OP_ADDR] = [this](const Instruction& instr) {
        if (instr.a < registers.size() && instr.b < registers.size() && instr.c < registers.size()) {
            registers[instr.a] = registers[instr.b] + registers[instr.c];
        }
    };

    dispatch[OP_LOADR] = [this](const Instruction& instr) {
        if (instr.a < registers.size()) {
            stack.push_back(registers[instr.a]);
        }
    };

    dispatch[OP_STORER] = [this](const Instruction& instr) {
        if (!stack.empty() && instr.a < registers.size()) {
            registers[instr.a] = stack.back();
            stack.pop_back();
        }
    };

    dispatch[OP_PRINT] = [this](const Instruction&) {
        if (!stack.empty()) {
            std::cout << stack.back() << std::endl;
        }
    };

    dispatch[OP_PRINTR] = [this](const Instruction& instr) {
        if (instr.a < registers.size()) {
            std::cout << "[PRINTR] R" << int(instr.a) << " = " << registers[instr.a] << std::endl;
        }
    };

    dispatch[OP_CMP] = [this](const Instruction& instr) {
    int a = (instr.a == 0xFF) ? counter : registers[instr.a];
    int b = (instr.b == 0xFF) ? counter : registers[instr.b];

    flag_eq = (a == b);
    flag_gt = (a > b);
    flag_lt = (a < b);

    std::cout << "[CMP] " << a << " vs " << b
              << " => EQ: " << flag_eq
              << ", GT: " << flag_gt
              << ", LT: " << flag_lt << std::endl;
    };

    dispatch[OP_JEQ] = [this](const Instruction& instr) {
    if (flag_eq) pc = instr.a - 1;
    };
    dispatch[OP_JNE] = [this](const Instruction& instr) {
        if (!flag_eq) pc = instr.a - 1;
    };
    dispatch[OP_JGT] = [this](const Instruction& instr) {
        if (flag_gt) pc = instr.a - 1;
    };
    dispatch[OP_JLT] = [this](const Instruction& instr) {
        if (flag_lt) pc = instr.a - 1;
    };


    dispatch[OP_LOADM] = [this](const Instruction& instr) {
    if (isValidAddr(instr.a)) {
        stack.push_back(memory[instr.a]);
        std::cout << "[LOADM] memory[" << instr.a << "] => " << memory[instr.a] << std::endl;
        }
    };

    dispatch[OP_STOREM] = [this](const Instruction& instr) {
        if (!stack.empty() && isValidAddr(instr.a)) {
            int val = stack.back(); stack.pop_back();
            memory[instr.a] = val;
            std::cout << "[STOREM] memory[" << instr.a << "] = " << val << std::endl;
        }
    };

    dispatch[OP_LOADMR] = [this](const Instruction& instr) {
    if (instr.a < registers.size() && isValidAddr(instr.b)) {
        registers[instr.a] = memory[instr.b];
        std::cout << "[LOADMR] R" << instr.a << " = memory[" << instr.b << "] = " << memory[instr.b] << std::endl;
        }
    };

    dispatch[OP_STOREMR] = [this](const Instruction& instr) {
        if (instr.b < registers.size() && isValidAddr(instr.a)) {
            memory[instr.a] = registers[instr.b];
            std::cout << "[STOREMR] memory[" << instr.a << "] = " << registers[instr.b] << std::endl;
        }
    };

    dispatch[OP_DECR] = [this](const Instruction&) {
        counter--;
    };

    dispatch[OP_CPRINT] = [this](const Instruction&) {
        std::cout << "[CPRINT] counter = " << counter << std::endl;
    };


    dispatch[OP_CEASE] = [this](const Instruction&) {
        pc = bytecode.size();  // force loop exit in runBytecode
    };
}



bool VirtualMachine::isValidAddr(int addr) const {
    return addr >= 0 && addr < memory.size();
}

void VirtualMachine::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        instructions.push_back(line);
    }
}

void VirtualMachine::run() {
    for (pc = 0; pc < instructions.size(); ++pc) {
        execute(instructions[pc]);
    }
}

void VirtualMachine::execute(const std::string& instrLine) {
    std::istringstream iss(instrLine);
    std::string instr;
    int operand;
    iss >> instr;

     if (!instr.empty() && instr.back() == ':') {
        // This is a label definition, skip execution
        return;
    }

    // ─── Control Flow ───
    if (instr == "HOP") { // move lines 
        iss >> operand;
        pc = operand - 1;
    }
    else if (instr == "CEASE") {
        pc = instructions.size(); // ends loop/program
    }

    // ─── Stack-Based ───
    else if (instr == "PUSH") { // psuh value onto stack
        iss >> operand;
        stack.push_back(operand);
    }
    else if (instr == "ADD") { // add two values on stack
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a + b);
    }
    else if (instr == "SUB") { // subtract tow values on stack
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a - b);
    }
    else if (instr == "MUL") { // multiply two values on stack
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a * b);
    }
    else if (instr == "DUP") { // duplicate a value on stack
        int top = stack.back();
        stack.push_back(top);
    }
    else if (instr == "PRINT") { // print a value on the stack
        std::cout << stack.back() << std::endl;
    }
    else if (instr == "PKPRINT") { /// peekprint: print a value wihtout affecting stack
        std::cout << stack.back() << std::endl;
    }
    else if (instr == "HNZ") { // if top of stack is not zero, hop to label
        int target;
        iss >> target;
        int value = stack.back(); stack.pop_back();
        if (value != 0) pc = target - 1;
    }
    else if (instr == "HZ") { // if top of stack is zero, hop to label
        int target;
        iss >> target;
        int value = stack.back(); stack.pop_back();
        if (value == 0) pc = target - 1;
    }

    // ─── Counter-Based ───
    else if (instr == "LOAD") { // load a value into counter
        iss >> counter;
    }
    else if (instr == "DECR") { // decrement counter
        counter--;
    }
    else if (instr == "CPRINT") { // print counter value
        std::cout << counter << std::endl;
    }
    else if (instr == "CHNZ") { // if counter is not zero, hop to label
        int target;
        iss >> target;
        if (counter != 0) pc = target - 1;
    }

    else if (instr == "STOREM") {
    int addr;
    iss >> addr;
    if (isValidAddr(addr)) {
    int value = stack.back(); stack.pop_back();
    memory[addr] = value;
    std::cout << "[STOREM] memory[" << addr << "] = " << value << std::endl;
    } else {
    std::cerr << "Invalid memory address in STOREM: " << addr << std::endl;
    std::exit(1);
        }
    }

    else if (instr == "LOADM") {
    int addr;
    iss >> addr;
    if (isValidAddr(addr)) {
    stack.push_back(memory[addr]);
    std::cout << "[LOADM] memory[" << addr << "] => " << memory[addr] << std::endl;
        }
    }

    else if (instr == "SETM") {
    int addr, val;
    iss >> addr >> val;
    if (isValidAddr(addr)) {
    memory[addr] = val;
    std::cout << "[SETM] memory[" << addr << "] = " << val << std::endl;
    } else {
        std::cerr << "Invalid memory address in SETM: " << addr << std::endl;
        std::exit(1);
        }
    }


    else if (instr == "MEMDUMP") {
    for (int i = 0; i < memory.size(); ++i) {
        if (memory[i] != 0)
            std::cout << "[" << i << "] = " << memory[i] << std::endl;
        }
    }


    else if (instr == "MOV") {
    std::string regName;
    int val;
    iss >> regName >> val;
    int regIndex = getRegisterIndex(regName);
    if (regIndex >= 0 && regIndex < registers.size()) {
        registers[regIndex] = val;
    } else {
        std::cerr << "Invalid register: " << regName << std::endl;
        }
    }


    else if (instr == "LOADR") {
        std::string regName;
        iss >> regName;
        int regIndex = getRegisterIndex(regName);
        if (regIndex >= 0 && regIndex < registers.size()) {
            stack.push_back(registers[regIndex]);
        } else {
            std::cerr << "Invalid register: " << regName << std::endl;
        }
    }


    else if (instr == "STORER") {
    std::string regName;
    iss >> regName;
    int regIndex = getRegisterIndex(regName);
    if (regIndex >= 0 && regIndex < registers.size()) {
        int value = stack.back(); stack.pop_back();
        registers[regIndex] = value;
    } else {
        std::cerr << "Invalid register: " << regName << std::endl;
        }
    }


    else if (instr == "ADDR") {
    std::string dest, src1, src2;
    iss >> dest >> src1 >> src2;
    int d = getRegisterIndex(dest);
    int s1 = getRegisterIndex(src1);
    int s2 = getRegisterIndex(src2);
    if (d >= 0 && s1 >= 0 && s2 >= 0) {
        registers[d] = registers[s1] + registers[s2];
    } else {
        std::cerr << "Invalid register in ADDR" << std::endl;
        }
    }


    else if (instr == "PRINTR") {
    std::string regName;
    iss >> regName;
    int r = getRegisterIndex(regName);
    if (r >= 0) {
        std::cout << "[PRINTR] " << regName << " = " << registers[r] << std::endl;
        }
    }


    else if (instr == "LOADMR") {
    std::string regName;
    int addr;
    iss >> regName >> addr;
    int r = getRegisterIndex(regName);
    if (r >= 0 && isValidAddr(addr)) {
        registers[r] = memory[addr];
        std::cout << "[LOADMR] " << regName << " = memory[" << addr << "] = " << memory[addr] << std::endl;
    } else {
        std::cerr << "Invalid LOADMR instruction." << std::endl;
        }
    }


    else if (instr == "STOREMR") {
    int addr;
    std::string regName;
    iss >> addr >> regName;
    int r = getRegisterIndex(regName);
    if (r >= 0 && isValidAddr(addr)) {
        memory[addr] = registers[r];
        std::cout << "[STOREMR] memory[" << addr << "] = " << registers[r] << std::endl;
    } else {
        std::cerr << "Invalid STOREM instruction." << std::endl;
        }
    }


    else if (instr == "CMP") {
    std::string left, right;
    iss >> left >> right;

    int a, b;
    if (left == "COUNTER") {
        a = counter;
    } else {
        int i = getRegisterIndex(left);
        if (i < 0) { std::cerr << "Invalid CMP reg: " << left << std::endl; return; }
        a = registers[i];
    }

    if (right == "COUNTER") {
        b = counter;
    } else {
        int i = getRegisterIndex(right);
        if (i < 0) { std::cerr << "Invalid CMP reg: " << right << std::endl; return; }
        b = registers[i];
    }

    flag_eq = (a == b);
    flag_gt = (a > b);
    flag_lt = (a < b);

    std::cout << "[CMP] " << left << "(" << a << ") vs " << right << "(" << b << ") => "
              << "EQ: " << flag_eq << ", GT: " << flag_gt << ", LT: " << flag_lt << std::endl;
    }


    else if (instr == "JEQ") {
    int target;
    iss >> target;
    if (flag_eq) pc = target - 1;
    }


    else if (instr == "JNE") {
    int target;
    iss >> target;
    if (!flag_eq) pc = target - 1;
    }


    else if (instr == "JGT") {
    int target;
    iss >> target;
    if (flag_gt) pc = target - 1;
    }


    else if (instr == "JLT") {
    int target;
    iss >> target;
    if (flag_lt) pc = target - 1;
    }





//EVENTUALLY ADD UI !!!1!



}

