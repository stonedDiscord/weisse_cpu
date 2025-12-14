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
KDC_DATA:       EQU 50H
KDC_CMD:        EQU 51H

WRITE_DISPLAY:  EQU 80H
CLEAR_DISPLAY:  EQU 0C1H
END_INT:        EQU 0E0H
READ_FIFO:      EQU 40H

; Initialize KDC
KDCINI: MVI     A, 0CH  ; DD MODE 1 KKK MODE 4
        OUT     KDC_CMD
        MVI     A, 3EH  ; CLOCK 102 KHZ
        OUT     KDC_CMD
        MVI     A, CLEAR_DISPLAY
        OUT     KDC_CMD
        MVI     A, END_INT
        OUT     KDC_CMD
        IN      KDC_CMD
        RET

; Read Sensor RAM
; Input: E = line number
RDKEY:  MOV     A,E
        ANI     07H ; clamp to 7
        MOV     E,A
        ORI     READ_FIFO ; build command
        OUT     KDC_CMD
        IN      KDC_DATA
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
; Input: C = digit number, D = mon, B = srv
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
MAINLP: MVI     A,END_INT
        OUT     KDC_CMD

        INR     E
        MOV     A, E
        ANI     07H     ; Mask to keep E in 0-7 range
        MOV     E, A

        CALL    RDKEY
        MOV     B, A

        MOV	A, E
        MOV	D, E

        MOV     C, E
        CALL    WR_DIG
        
        CALL    WRLAMP
        

        JMP     MAINLP
        HLT