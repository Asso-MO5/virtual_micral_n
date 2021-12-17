    ORG 0x0000
    LCC             ; MAS instruction of the Micral N: disables all interruptions except Level 0.

COPY:
    LBI 0
    LCB

COPY_LOOP:
    LHB
    LLC
    LAM

    INC
    JFZ NO_INC_B
    INB
NO_INC_B:
    INH     ; Points 0x0100 further
    LMA

    LDH     ; Prepare for end compariso

    LAI \LB\PATCH_H+2 ; Points to the CAL instruction address MSByte
    CPL
    JFZ NO_PATCH
    LAH     ; Gets the MSByte of the current copy
    LMA     ; Injects the call
    LLI \LB\PATCH_L+2
    LMA     ; Injects the call
    LLI \LB\LOOP+2
    LMA     ; Injects the call
NO_PATCH:
    LAD     ; Compare for end of loop
    CPI 0x37

    JFZ COPY_LOOP
    JMP LOOP

LOOP:
    CAL LOOP_CALL

    LHI \HB\IND_JMP+2
    LLI \LB\IND_JMP+2

    ; Bit loop
    LCI 0
    LAM
    ADA ; Doubles A
    ACC ; Adds the Carry in case of overflow
    LMA

    JMP IND_JMP

IND_JMP:
    DATA 0x44
    DATA \LB\LOOP
    DATA 0x01

LOOP_CALL:
    LHI 0x10
LOOP_H:
    LLI 0xFF
LOOP_L:
    DCL
PATCH_L:
    JFZ LOOP_L
    DCH
PATCH_H:
    JFZ LOOP_H
    RET


    ; Maximum address 0x7F
