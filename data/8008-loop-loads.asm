    ORG 0x000

    LAA
LOOP:
    LAI 1
    LBI 2
    LCI 4
    LDI 8
    LEI 16
    LHI 32
    LLI 64
    LAI 128
    LLI 64
    LHI 32
    LEI 16
    LDI 8
    LCI 4
    LBI 2
    JMP LOOP


