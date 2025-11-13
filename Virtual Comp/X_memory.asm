; Memory load/store demo (via registers)

MOV R0 42
STOREMR 20 R0     ; MEM[20] = 42
LOADMR R1 20      ; R1 = 42
PRINTR R1         ; expect: [PRINTR] R1 = 42
CEASE
