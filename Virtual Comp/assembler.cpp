#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <regex>

struct Instruction {
    uint8_t opcode, a, b, c;
};

static const std::unordered_map<std::string, uint8_t> opcodeMap = {
    {"PUSH", 0x01}, {"MOV", 0x02}, {"ADDR", 0x03}, {"SUBR", 0x04},
    {"LOADR", 0x05}, {"STORER", 0x06}, {"PRINT", 0x07}, {"PRINTR", 0x08},
    {"CMP", 0x09}, {"JEQ", 0x0A}, {"JNE", 0x0B}, {"JGT", 0x0C}, {"JLT", 0x0D},
    {"LOADM", 0x0E}, {"STOREM", 0x0F}, {"LOADMR", 0x10}, {"STOREMR", 0x11},
    {"DECR", 0x12}, {"CPRINT", 0x13}, {"CEASE", 0x14}
};

static bool isInt(const std::string& s) {
    if (s.empty()) return false;
    char* end=nullptr;
    long v = strtol(s.c_str(), &end, 10);
    (void)v;
    return end && *end == '\0';
}

static void stripComment(std::string& line) {
    auto p = line.find(';');
    if (p != std::string::npos) line = line.substr(0, p);
    // trim
    auto l = line.find_first_not_of(" \t\r\n");
    auto r = line.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) { line.clear(); return; }
    line = line.substr(l, r - l + 1);
}

int main() {
    std::ifstream input("instructions.txt");
    std::ofstream output("program.bin", std::ios::binary);
    if (!input || !output) { std::cerr << "Error opening file(s)\n"; return 1; }

    std::regex labelRegex(R"(^\s*([A-Za-z_.][\w.]*)\s*:\s*(.*)$)");
    std::unordered_map<std::string,int> labels;
    std::vector<std::vector<std::string>> prog;
    std::string line;
    int ip = 0; // instruction index (0-based)

    // -------- First pass: collect labels and normalized token lines --------
    while (std::getline(input, line)) {
        stripComment(line);
        if (line.empty()) continue;

        std::smatch m;
        if (std::regex_match(line, m, labelRegex)) {
            std::string name = m[1];
            std::string rest = m[2];
            labels[name] = ip + 1; // 1-based to match VM jump pc = target-1
            line = rest;
            // after label, rest may be empty or an instruction
            stripComment(line);
            if (line.empty()) continue;
        }

        // tokenize remaining instruction
        std::istringstream iss(line);
        std::vector<std::string> parts;
        std::string tok;
        while (iss >> tok) parts.push_back(tok);
        if (!parts.empty()) {
            prog.push_back(std::move(parts));
            ip++;
        }
    }

    // -------- Second pass: encode and write --------
    auto encodeOperand = [&](const std::string& tok) -> uint8_t {
        if (tok.empty()) return 0;
        if (tok == "COUNTER") return 0xFF;
        if (tok[0] == 'R' && tok.size() > 1 && isInt(tok.substr(1)))
            return static_cast<uint8_t>(std::stoi(tok.substr(1)));
        if (isInt(tok)) return static_cast<uint8_t>(std::stoi(tok));
        auto it = labels.find(tok);
        if (it != labels.end()) return static_cast<uint8_t>(it->second & 0xFF);
        std::cerr << "Unknown operand '" << tok << "' (not int/reg/label)\n";
        exit(2);
    };

    for (auto& parts : prog) {
        std::string op = parts[0];
        auto it = opcodeMap.find(op);
        if (it == opcodeMap.end()) {
            std::cerr << "Unknown instruction: " << op << "\n";
            continue;
        }
        Instruction instr{};
        instr.opcode = it->second;
        instr.a = parts.size() > 1 ? encodeOperand(parts[1]) : 0;
        instr.b = parts.size() > 2 ? encodeOperand(parts[2]) : 0;
        instr.c = parts.size() > 3 ? encodeOperand(parts[3]) : 0;
        output.write(reinterpret_cast<const char*>(&instr), sizeof(instr));
    }

    std::cout << "Assembled to program.bin\n";
    return 0;
}
