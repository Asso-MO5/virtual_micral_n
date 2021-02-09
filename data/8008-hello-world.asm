RAM: EQU 0x1000

START:
    ORG 0x0000
    LAA ; Starts with a NOP
    ; DE points to text message
    LDI \HB\TEST_MSG-1
    LEI \LB\TEST_MSG-1

    ; BC points to RAM
    LBI \HB\RAM-1
    LCI \LB\RAM-1

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
    LHC
    CAL ADVANCE_HL
    LBH
    LCH

    LMA ; Write BYTE

    ORA
    JFZ LOOP
    HLT
    JMP START

ADVANCE_HL:
    INL
    RFZ
    INH
    RET

TEST_MSG:
    DATA "HELLO WORLD"
    DATA 0
