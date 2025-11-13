; Counter demo (no direct load exists in bytecode, just show DECR/CPRINT)

CPRINT            ; expect: [CPRINT] counter = 0
DECR
CPRINT            ; expect: [CPRINT] counter = -1
CEASE
