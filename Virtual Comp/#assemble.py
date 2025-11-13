# assemble.py
import struct
import re

opcode_map = {
    "PUSH":    0x01,
    "MOV":     0x02,
    "ADDR":    0x03,
    "SUBR":    0x04,
    "LOADR":   0x05,
    "STORER":  0x06,
    "PRINT":   0x07,
    "PRINTR":  0x08,
    "CMP":     0x09,
    "JEQ":     0x0A,
    "JNE":     0x0B,
    "JGT":     0x0C,
    "JLT":     0x0D,
    "LOADM":   0x0E,
    "STOREM":  0x0F,
    "LOADMR":  0x10,
    "STOREMR": 0x11,
    "DECR":    0x12,
    "CPRINT":  0x13,
    "CEASE":   0x14,
}

label_def = re.compile(r'^\s*([A-Za-z_.][\w.]*)\s*:\s*(.*)$')

def tokenize(line: str):
    # strip comments and whitespace
    line = line.split(';', 1)[0].strip()
    if not line:
        return None, []
    # support "label: instr ..." on one line
    m = label_def.match(line)
    label = None
    if m:
        label = m.group(1)
        line = m.group(2).strip()
    parts = line.split() if line else []
    return label, parts

def is_int(s: str):
    try:
        int(s)
        return True
    except:
        return False

def encode_operand(tok: str, labels: dict):
    if tok == "":
        return 0
    if tok == "COUNTER":
        return 0xFF
    if tok.startswith("R") and tok[1:].isdigit():
        return int(tok[1:])
    if is_int(tok):
        return int(tok)
    # label reference: use resolved address (already 1-based)
    if tok in labels:
        return labels[tok]
    raise ValueError(f"Unknown operand '{tok}' (not int/reg/label)")

# -------- First pass: collect labels and instruction lines --------
lines = []
labels = {}
ip = 0  # instruction index (0-based)

with open("instructions.txt") as infile:
    for raw in infile:
        label, parts = tokenize(raw)
        if label is not None:
            # label points to the NEXT instruction address
            labels[label] = ip + 1  # <- make it 1-based to match VM (pc = target-1)
        if parts:
            lines.append(parts)
            ip += 1

# -------- Second pass: emit bytecode --------
with open("program.bin", "wb") as outfile:
    for parts in lines:
        op_name = parts[0].upper()
        if op_name not in opcode_map:
            raise ValueError(f"Unknown instruction '{op_name}'")
        op = opcode_map[op_name]
        a = encode_operand(parts[1], labels) if len(parts) > 1 else 0
        b = encode_operand(parts[2], labels) if len(parts) > 2 else 0
        c = encode_operand(parts[3], labels) if len(parts) > 3 else 0
        outfile.write(struct.pack("BBBB", op, a & 0xFF, b & 0xFF, c & 0xFF))

print("Assembled to program.bin")
