; 8085 Assembly program for AS8085
; Target: 8279 Keyboard/Display Controller

.8085

; =========================
; Constants (column 1!)
; =========================
KDC_DATA        .EQU 0x50
KDC_CMD         .EQU 0x51
WRITE_DISPLAY   .EQU 0x80

; =========================
; Absolute code area
; =========================
.area CODE ABS
.org 0x200

        JMP START

; =========================
; Initialize 8279
; =========================
KDCINI:
        MVI A,0x0C
        OUT KDC_CMD
        MVI A,0x3E
        OUT KDC_CMD
        MVI A,0xC1
        OUT KDC_CMD
        MVI A,0xE0
        OUT KDC_CMD
        RET

; =========================
; Write to lamps
; =========================
WRLAMP:
        MOV A,C
        ORI WRITE_DISPLAY
        OUT KDC_CMD
        MOV A,B
        OUT KDC_DATA
        RET

; =========================
; Write to digits
; =========================
WR_DIG:
        MOV     A,C
        ADI     0x08
        ORI     WRITE_DISPLAY
        OUT     KDC_CMD
        MOV     A,B
        RLC
        RLC
        RLC
        RLC
        ORA     D
        OUT     KDC_DATA
        RET

; =========================
; Main program
; =========================
START:
        CALL    KDCINI

        MVI     B,0xFF
        MVI     C,0x00

LAMPLP:
        CALL    WRLAMP
        INR     C
        MOV     A,C
        CPI     0x08
        JNZ     LAMPLP

        MVI     B,0x03
        MVI     D,0x04
        MVI     C,0x00

DIGILP:
        CALL    WR_DIG
        INR     C
        MOV     A,C
        CPI     0x08
        JNZ     DIGILP

MAINLP:
        IN      KDC_CMD
        ANI     0x04
        JNZ     NOKEY

        IN      KDC_DATA
        MOV     B,A
        JMP     DISPKY

NOKEY:
        MVI     B,0x00

DISPKY:
        MOV A,B
        RRC
        RRC
        RRC
        RRC
        ANI     0x0F
        MOV     D,A
        MVI     C,0x00
        MVI     B,0x00
        CALL    WR_DIG

        MOV     A,B
        ANI     0x0F
        MOV     D,A
        MVI     C,0x01
        MVI     B,0x00
        CALL    WR_DIG

        JMP     MAINLP

        HLT
.end
