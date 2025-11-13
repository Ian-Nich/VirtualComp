#include <iostream>
#include "VirtualMachine.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout <<
"Usage:\n"
"  vm --run    program.bin [--trace] [--explain] [--bp N ...]\n"
"  vm --step   program.bin [--trace] [--explain] [--bp N ...]\n"
"  vm --disasm program.bin\n";
        return 0;
    }

    std::string mode = argv[1];
    std::string file = argv[2];

    VirtualMachine vm;

    std::vector<int> bps;
    for (int i = 3; i < argc; ++i) {
        std::string flag = argv[i];
        if (flag == "--trace") vm.setTrace(true);
        else if (flag == "--explain") vm.setExplain(true);
        else if (flag == "--bp" && i+1 < argc) {
            int n = std::stoi(argv[++i]);
            bps.push_back(n);
        }
    }
    vm.setBreakpoints(bps);

    vm.loadBytecode(file);

    if (mode == "--run") {
        vm.runBytecode();
    } else if (mode == "--step") {
        vm.runBytecodeStep();
    } else if (mode == "--disasm") {
        vm.disassemble();
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
    }
    return 0;
}
