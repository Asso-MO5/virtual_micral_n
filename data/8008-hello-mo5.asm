; Copy a string from a place to another.

RAM: EQU 0x1000

START:
    ORG 0x0000
    LAA ; Starts with a NOP
    LCC ; Mask interruptions

TXT_OUTPUT:
    LHI \HB\TEST_MSG
    LLI \LB\TEST_MSG

    CAL OUTPUT_STR
    HLT
    LAA ; Insert NOP in case no instruction is jammed
    JMP TXT_OUTPUT

ADVANCE_HL:
    INL
    RFZ
    INH
    RET

    ; Output of HL as a NULL terminated string to the Serial Card
OUTPUT_STR:
    LAM ; Read Byte
    ORA
    RTZ ; End of String
    CAL OUTPUT_CHAR
    CAL ADVANCE_HL
    JMP OUTPUT_STR

    ; Output of A content to the Serial Card
    ; Uses C
OUTPUT_CHAR:
    LCA
    ; Wait for Output Ready
    LAI 0x01
    INP 0x0
    RLC
    JFC OUTPUT_CHAR

    LAC
    OUT 0x0
    RET

TEST_MSG:
    DATA "HELLO MO5 !"
    DATA 13,10,0

