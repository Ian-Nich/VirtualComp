# VirtualComp
1st summer project where I built a hands-on CPU emulator that deepens understanding of computer organization and prepares me for upper-level CS courses.
---

# 🧠 Educational CPU Lab

### A Virtual CPU Emulator and Interactive Debugger Built in C++

This project implements a **stack and register-based virtual CPU** designed as an **educational tool** for learning how low-level systems, instruction sets, and debuggers work.
It features a full instruction execution pipeline, bytecode format, assembler, and interactive step-through debugger with tracing and breakpoints.

---

## 🚀 Features

* **Custom Virtual CPU Architecture**

  * Stack operations (`PUSH`, `POP`, `ADD`, `SUB`, etc.)
  * 8 general-purpose registers `R0–R7`
  * Arithmetic, memory, and control-flow instructions
  * Flags for comparisons (`EQ`, `GT`, `LT`)
  * `COUNTER` register for loop control

* **Assembler → Bytecode → Emulator Flow**

  * Write readable assembly (e.g. `MOV R0 5`, `CMP R0 R1`, `JGT loop`)
  * Assemble to `.bin` format using a Python or C++ assembler
  * Run in the VM and observe register/memory changes

* **Interactive Stepper Debugger**

  * `step`, `cont`, and `breakpoint` commands
  * `trace` and `explain` toggles for full or human-readable execution output
  * Real-time register, memory, and stack inspection
  * Built-in disassembler and help system

---

## 🧩 Project Structure

```
.
├── VirtualMachine.h       # CPU class definition
├── VirtualMachine.cpp     # Execution engine + debugger
├── main.cpp               # CLI and argument parsing
├── assembler.cpp / .py    # Source-to-bytecode assembler
├── instructions.txt        # Example assembly source
├── test.bin               # Compiled bytecode example
└── README.md
```

---

## ⚙️ Build & Run

### 1. Compile

```bash
g++ -std=c++17 -o vm main.cpp VirtualMachine.cpp
```

### 2. Run Normally

```bash
./vm program.bin
```

### 3. Run With Debug Tools

```bash
./vm --trace --explain program.bin
```

### 4. Run Step-by-Step Debugger

```bash
./vm --step program.bin
```

Inside the REPL:

```
(vm) help
(vm) step
(vm) regs
(vm) cont
(vm) bp add 3
(vm) bp del 3
(vm) bp list
(vm) bp clear
(vm) quit
```

---

## 🧮 Example Program

### instructions.txt

```
MOV R0 5
MOV R1 0
PRINTR R0
DECR
CMP COUNTER R1
JGT 2     ; loop back to PRINTR
CEASE
```

### Expected Output

```
[PRINTR] R0 = 5
[CMP] COUNTER(-1) vs R1(0) => EQ: 0, GT: 0, LT: 1
```

---

## 🧠 Educational Goals

The **Educational CPU Lab** simulates the evolution of a CPU emulator:

| Level | Concept      | What You Build                 |
| :---: | ------------ | ------------------------------ |
|   1   | Stack VM     | Simple interpreter             |
|   2   | Flow Control | Loops, IFs                     |
|   3   | Memory       | Read/write to RAM              |
|   4   | Registers    | Register-based ops             |
|   5   | Bytecode     | Binary instruction format      |
|   6   | Assembly     | Assembler + disassembler       |
|   7   | Pipeline     | Stepper + debugger             |
|   8   | Real CPU     | Extend to 6502, Z80, or RISC-V |

---

## 🧩 Future Extensions

* I/O Devices (keyboard, display, timers)
* Instruction timing and pipeline simulation
* Multi-program or multitasking simulation
* Integration with a web-based UI or VSCode extension
* Real ISA emulation (6502, Z80, or RISC-V subset)

---

## 📘 Example Use Case

**Goal:** Create an interactive teaching tool for learning CPU architecture, instruction sets, and debugging.

Students can:

* Write programs in custom assembly
* Watch CPU state evolve step-by-step
* Experiment with branching, loops, and memory operations
* Extend the VM to simulate additional hardware components

---

## 🧑‍💻 Author

Developed by **Ian Nicholas-Flerin**
Built with C++17 for educational and experimental use.

---

## 🧾 License

MIT License — freely available for learning, modification, and distribution.

---
