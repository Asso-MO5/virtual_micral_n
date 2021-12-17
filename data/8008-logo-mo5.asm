    ; This program is loaded at address 0 and will display a MO5.COM banner
    ; on the serial output of the Micral N
    ;
    ; Commented to give a glipse of programming the Micral N
    ; Only uses a Serial Card on Output port 0.
    ; No Stack Channel Card needed.
    ;
    ; Written by Sylvain Glaize, 2021
    ; Released to the public domain.

    ORG 0x0000
    LCC             ; MAS instruction of the Micral N: disables all interruptions except Level 0.
    JMP START

    ORG 0x0038
RST7:
    DATA 0x1f ; REI

START:
    LHI \HB\LOGO    ; Loads the address of LOGO in HL
    LLI \LB\LOGO    ; There is no register pair operation on the 8008.

    ; This is the main loop of uncompressing and displaying the LOGO
LOOP:
    LAM             ; Load a byte from the memory pointed by HL into register A.

    ORA             ; Updates flag
    JTZ STR         ; Jumps if Z is true, which means A was equal to 0. This is the final byte of
                    ; the encoded data.

    LBA             ; Saves register A to register B

    ; A contains the coded character in the form of CCcccccc, where CC is the character index on
    ; 2 bits and cccccc is the count on 6 bits.
    LCI 0x3F        ; Loads register C with the mask for the 6 lower bits.
    NDC             ; Bitwise ANDs between C and A, to keep the lowest 6 bits of the coded data.
    LCA             ; Loads the result into register C, which now contains the character repetition
                    ; count to display.

    LDH             ; Saves HL to DE
    LEL             ; ...

    LAB             ; Gets back the ORDginal coded character from register B, where is was
                    ; saved before.

    ; The character index to display will now be extracted from the two high bits of A.
    LBI 0xC0        ; Loads register B with the mask for the two higher bits.
    NDB             ; Bitwise ANDs between B and A, to keep the higher 2 bits of the coded data.
    RLC             ; Rotates the two higher bits...
    RLC             ; ... into the two lower bits of register A.

    LHI \HB\CHAR    ; Make HL point to the CHAR array.
    LLI \LB\CHAR    ;
    ADL             ; Adds the index to the lower part of HL (which means this will not work if
    LLA             ; HL points at the end of a page. In that case, the Carry should ripple to H.
    LAM             ; Reads the character from the array.

    LHD             ; Gets back HL pointing on the encoded data
    LLE             ; from DE where is was previously saved.

OUTPUT:
    CAL OUTPUT_CHAR ; Outputs the character in A on the serial card
    DCC             ; Decreases the counter in the register C
    JFZ OUTPUT      ; If C is not Zero, the loops to display the character again.
                    ; If C was Zero, continues.
    CAL ADVANCE_HL  ; Moves HL to point to the next coded byte.
    JMP LOOP        ; And loops.

    ; When the encoded banner is displayed, the program continues here.
    ; It will now display the null terminal string which follows the encoded data
STR:
    CAL ADVANCE_HL  ; HL was pointing to the 0x00 at the end of DATA. Moving it to the next byte.

OUTPUT_STR:
    LAM             ; Gets the byte pointer by HL
    ORA             ; Updates the flags
    JTZ END         ; Jumps to END if the Zero flag was set
                    ; A was equal to 0x00, which marks the end of the string.
    CAL OUTPUT_CHAR ; Else, Outputs the byte A as a character.
    CAL ADVANCE_HL  ; Moves HL to the next byte.
    JMP OUTPUT_STR  ; And loops for the next character.

END:
    JMP BLINK
    HLT             ; Stops all operations.

    ; Output of A content to the Serial Card
    ; Uses B as an auxiliary register.
OUTPUT_CHAR:
    LBA             ; Saves the register A, parameter of the routine, in register B.

W_OUTPUT_R:
    LAI 0x01        ; Reads from the Input at sub-address 0x01...
    INP 0x0         ; ... of the input address 0x00
    RLC             ; Pushes the higher bit into Carry
    JFC W_OUTPUT_R  ; If the Carry is not set (the higher bit was 0), it means the Serial Card
                    ; is not ready for Output. So, loops.

    LAB             ; Gets back the parameter saved in register B.
    OUT 0x0         ; Outputs the character to the Serial card.
    RET             ; And returns from the routine.

    ; This routines implements `INC HL`, which doesn't exist on the 8008
ADVANCE_HL:
    INL             ; First, increment L
    RFZ             ; Returns if L is not zero.
    INH             ; Else, it means L caused a carry, increment H.
                    ; The "increment" and "decrement" operations do not affect the Carry flag on
                    ; the 8008.
    RET             ; And returns.

    ORG 0x100
BLINK:
    LDI 0x0
    LAI 0x0
    JMP BL_200

BL_100:
    LHI 0x10
W_100_H:
    LLI 0xFF
W_100_L:
    DCL
    JFZ W_100_L
    DCH
    JFZ W_100_H

    ORD
    JTZ BL_200
    JMP BLINK

    ORG 0x200
BL_200:
    LHI 0x10
W_200_H:
    LLI 0xFF
W_200_L:
    DCL
    JFZ W_200_L
    DCH
    JFZ W_200_H
    
    ORD
    JTZ BL_400
    JMP BL_100

CHAR:
    DATA " .#"      ; This is the array of displayable characters.
    DATA 0x0d       ; ...

    ; Following is the encoded data representing the MO5.COM banner in ASCII Art.
    ; Each byte is coded in the form of CCcccccc, where CC is the character index on 2 bits,
    ; indexing the array just above, and cccccc is the count on 6 bits.
LOGO:
    DATA 0x1, 0x82, 0x44, 0x1, 0x82, 0x42, 0x1, 0x87, 0x42, 0x1, 0x88, 0x47
    DATA 0x1, 0x86, 0x43, 0x1, 0x87, 0x42, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41
    DATA 0xc1, 0x1, 0x83, 0x42, 0x1, 0x83, 0x41, 0x1, 0x82, 0x44, 0x1, 0x82
    DATA 0x41, 0x1, 0x82, 0x4c, 0x1, 0x82, 0x43, 0x1, 0x82, 0x41, 0x1, 0x82
    DATA 0x44, 0x1, 0x82, 0x41, 0x1, 0x83, 0x42, 0x1, 0x83, 0x41, 0xc1, 0x1
    DATA 0x84, 0x1, 0x84, 0x41, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41, 0x1, 0x82
    DATA 0x4c, 0x1, 0x82, 0x47, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41, 0x1, 0x84
    DATA 0x1, 0x84, 0x41, 0xc1, 0x1, 0x82, 0x1, 0x83, 0x1, 0x82, 0x41, 0x1
    DATA 0x82, 0x44, 0x1, 0x82, 0x41, 0x1, 0x87, 0x47, 0x1, 0x82, 0x47, 0x1
    DATA 0x82, 0x44, 0x1, 0x82, 0x41, 0x1, 0x82, 0x1, 0x83, 0x1, 0x82, 0x41
    DATA 0xc1, 0x1, 0x82, 0x41, 0x1, 0x81, 0x41, 0x1, 0x82, 0x41, 0x1, 0x82
    DATA 0x44, 0x1, 0x82, 0x47, 0x1, 0x82, 0x46, 0x1, 0x82, 0x47, 0x1, 0x82
    DATA 0x44, 0x1, 0x82, 0x41, 0x1, 0x82, 0x41, 0x1, 0x81, 0x41, 0x1, 0x82
    DATA 0x41, 0xc1, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41, 0x1, 0x82, 0x44, 0x1
    DATA 0x82, 0x41, 0x1, 0x82, 0x43, 0x1, 0x82, 0x41, 0x1, 0x83, 0x41, 0x1
    DATA 0x82, 0x43, 0x1, 0x82, 0x41, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41, 0x1
    DATA 0x82, 0x44, 0x1, 0x82, 0x41, 0xc1, 0x1, 0x82, 0x44, 0x1, 0x82, 0x42
    DATA 0x1, 0x87, 0x43, 0x1, 0x86, 0x42, 0x1, 0x83, 0x42, 0x1, 0x86, 0x43
    DATA 0x1, 0x87, 0x42, 0x1, 0x82, 0x44, 0x1, 0x82, 0x41, 0xc1, 0x00

    ; Following the encoded data is a plain null terminated string that
    ; will be emitted as-is
    DATA " PRESERVER  EXPLORER   REJOUER"
    DATA 13,0



    ORG 0x400
BL_400:
    LHI 0x10
W_400_H:
    LLI 0xFF
W_400_L:
    DCL
    JFZ W_400_L
    DCH
    JFZ W_400_H
    
    ORD
    JTZ BL_800
    JMP BL_200

    ORG 0x800
BL_800:
    LHI 0x10
W_800_H:
    LLI 0xFF
W_800_L:
    DCL
    JFZ W_800_L
    DCH
    JFZ W_800_H
    
    ORD
    JTZ BL_1000
    JMP BL_400

    ORG 0x1000
BL_1000:
    LHI 0x10
W_1000_H:
    LLI 0xFF
W_1000_L:
    DCL
    JFZ W_1000_L
    DCH
    JFZ W_1000_H
    
    ORD
    JTZ BL_2000
    JMP BL_800

    ORG 0x2000
BL_2000:
    LHI 0x10
W_2000_H:
    LLI 0xFF
W_2000_L:
    DCL
    JFZ W_2000_L
    DCH
    JFZ W_2000_H
    
    LDI 0x1
    JMP BL_1000


;;; Following is the Python source code to encode the banner.
;;;
;;; Just remove the ;;; at the beginning of the line (including the space)
;;; At the end of the DATA, add a 0x00 to mark the end.

;;; logo = """
;;;  ##.... ##.. #######.. ########....... ######... #######.. ##.... ##.
;;;  ###.. ###. ##.... ##. ##............ ##... ##. ##.... ##. ###.. ###.
;;;  #### ####. ##.... ##. ##............ ##....... ##.... ##. #### ####.
;;;  ## ### ##. ##.... ##. #######....... ##....... ##.... ##. ## ### ##.
;;;  ##. #. ##. ##.... ##....... ##...... ##....... ##.... ##. ##. #. ##.
;;;  ##.... ##. ##.... ##. ##... ##. ###. ##... ##. ##.... ##. ##.... ##.
;;;  ##.... ##.. #######... ######.. ###.. ######... #######.. ##.... ##.
;;; """
;;;
;;;
;;; def code_char(char):
;;;     return " .#\x0D".find(str(char))
;;;
;;;
;;; def decode_char(c):
;;;     return " .#\x0D"[c]
;;;
;;;
;;; coded = []
;;;
;;; for line in logo.split("\n"):
;;;     if not line:
;;;         continue
;;;
;;;     counter = 0
;;;     character = ''
;;;     for c in line:
;;;         if c != character:
;;;             coded.append((counter, character))
;;;             counter = 1
;;;             character = c
;;;         else:
;;;             counter += 1
;;;
;;;     coded.append((counter, character))
;;;     coded.append((1, 13))
;;;
;;;
;;; coded = [(count & 0x3F | code_char(char) << 6) & 0xff
;;;          for (count, char) in coded if count > 0]
;;;
;;; hex_coded = [hex(c) for c in coded]
;;;
;;; width = 12
;;; output = [hex_coded[i:i + width] for i in range(0, len(hex_coded), width)]
;;;
;;; for o in output:
;;;     print("DATA " + ", ".join([h for h in o]))
