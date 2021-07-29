; Copy a string from a place to another.

RAM: EQU 0x1000

START:
    ORG 0x0000
    LAA ; Starts with a NOP
    LCC ; Mask interruptions

    ; Output the text
    LHI \HB\TEST_MSG
    LLI \LB\TEST_MSG
    CAL OUTPUT_STR

    ; Input command
COMMAND:
    CAL INPUT_CHAR
    CPI 82 ; 'R'
    JTZ 0x0000
    CPI 77 ; 'M'
    JTZ 0x3B30
    JMP COMMAND

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

W_OUTPUT_R:
    ; Wait for Output Ready
    LAI 0x01
    INP 0x0
    RLC
    JFC W_OUTPUT_R

    LAC
    OUT 0x0
    RET

    ; Input from the Serial Card to A
INPUT_CHAR:
    ; Wait for Input Ready
    LAI 0x01
    INP 0x0
    RRC
    JFC INPUT_CHAR

    ; Reads a value different from -xFF
    XRA
    INP 0x0
    CPI 0xff
    JTZ INPUT_CHAR
    RET

TEST_MSG:
    DATA "HELLO MO5 !"
    DATA 13,10
    DATA "(M) to go to Monitor"
    DATA 13,10
    DATA "(R) to reset"
    DATA 13,10
    DATA 0
