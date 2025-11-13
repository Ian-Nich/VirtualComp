; Control-Flow demo with CMP/JEQ/JNE/JGT/JLT
; We set flags using CMP and demonstrate a conditional jump.

MOV R0 1
MOV R1 1
CMP R0 R1         ; sets EQ=1
JEQ 8             ; jump to line 8 if EQ
PRINTR R0         ; (will be skipped because JEQ taken)
CEASE

; target of JEQ
PRINTR R1         ; expect: [PRINTR] R1 = 1
CEASE
