; Copy a string from a place to another.

RAM: EQU 0x1000

START:
    ORG 0x0000
    LAA ; Starts with a NOP

    ; BC points to RAM
    LBI \HB\RAM-1
    LCI \LB\RAM-1

TXT_COPY:
    ; DE points to text message
    LDI \HB\TEST_MSG-1
    LEI \LB\TEST_MSG-1

LOOP:
    ; Advances read pointer
    LHD
    LLE
    CAL ADVANCE_HL
    LDH 
    LEL

    LAM ; Read Byte

    ; Write byte to RAM and advances pointer
    LHB
    LLC
    CAL ADVANCE_HL
    LBH
    LCL

    LMA ; Write BYTE

    ORA
    JFZ LOOP

    LAH
    CPI 0x20
    JFZ TXT_COPY
    HLT
    LAA ; Insert NOP in case no instruction is jammed

ADVANCE_HL:
    INL
    RFZ
    INH
    RET

TEST_MSG:
    DATA "HELLO MO5 !"
    DATA 0

