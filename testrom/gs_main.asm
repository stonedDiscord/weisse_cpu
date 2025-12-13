; 8085 Assembly program for GNUSim8085
; Target: 8279 Keyboard/Display Controller

; IO Ports:
; 0x50 - 0x51 8279
; 0x60 - 0x6f 8256
; 0x72        Sound

;RST0
DI
LXI     SP, 0C7FCH
NOP
JMP     reset
;RST1
JMP     start
NOP
JMP     start
NOP
;RST2
JMP     start
NOP
JMP     start
NOP
;RST3
JMP     start
NOP
JMP     start
NOP
;RST4
JMP     start
NOP
;TRAP
JMP     start
NOP
;RST5
JMP     start
NOP
;RST5.5
JMP     start
NOP
;RST6
JMP     start
NOP
;RST6.5
JMP     start
NOP
;RST7
JMP     start
NOP
;RST7.5
JMP     start
NOP

;data
KDC_DATA:      EQU 50H
KDC_CMD:       EQU 51H
WRITE_DISPLAY: EQU 80H

; Initialize KDC
KDCINI: MVI     A, 0CH
        OUT     KDC_CMD
        MVI     A, 3EH
        OUT     KDC_CMD
        MVI     A, 0C1H
        OUT     KDC_CMD
        MVI     A, 0E0H
        OUT     KDC_CMD
        RET

; Write to lamps
; Input: C = line number, B = data
WRLAMP: MOV     A, C
        ORI     WRITE_DISPLAY
        OUT     KDC_CMD
        MOV     A, B
        OUT     KDC_DATA
        RET

; Write to digits
; Input: C = digit number, B = mon, D = srv
WR_DIG: MOV     A, C
        ADI     8
        ORI     WRITE_DISPLAY
        OUT     KDC_CMD
        MOV     A, B
        RLC
        RLC
        RLC
        RLC
        ORA     D
        OUT     KDC_DATA
        RET

; Main program
reset:  CALL    KDCINI
        ; Turn on all lamps
start:  MVI     B, 0FFH
        MVI     C, 0
LAMPLP: CALL    WRLAMP
        INR     C
        MOV     A, C
        CPI     8
        JNZ     LAMPLP
        ; Initialize all digits
        MVI     B, 03H
        MVI     D, 04H
        MVI     C, 0
DIGILP: CALL    WR_DIG
        INR     C
        MOV     A, C
        CPI     8
        JNZ     DIGILP
        ; Infinite loop to scan keyboard
MAINLP: IN      KDC_CMD
        ANI     04H
        JNZ     NOKEY
        ; Key pressed - read it
        IN      KDC_DATA
        MOV	B, A
        JMP     DISPKY
NOKEY:  MVI	B, 0
DISPKY: MOV	A, B
        RRC
        RRC
        RRC
        RRC
        ANI     0FH
        MOV     D, A
        MVI     C, 0
        MVI     B, 0
        CALL    WR_DIG
        MOV     A, B
        ANI     0FH
        MOV     D, A
        MVI     C, 1
        MVI     B, 0
        CALL    WR_DIG
        JMP     MAINLP
        HLT