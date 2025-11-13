; Arithmetic & Registers demo
; Shows MOV, ADDR, PRINTR, stack PUSH/PRINT, and memory register I/O.

MOV R0 7          ; R0 = 7
MOV R1 5          ; R1 = 5
ADDR R2 R0 R1     ; R2 = R0 + R1 = 12
PRINTR R2         ; expect: [PRINTR] R2 = 12

; Memory <-> Register roundtrip
STOREMR 10 R2     ; MEM[10] = R2 (12)
LOADMR R3 10      ; R3 = MEM[10] (12)
PRINTR R3         ; expect: [PRINTR] R3 = 12

; Stack print (push a literal then PRINT top of stack)
PUSH 99
PRINT             ; expect: 99

CEASE
