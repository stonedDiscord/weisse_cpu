; 8085 Assembly program for GNUSim8085
; Target: Note Player Test
; Based on gs_main.asm

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

; Constants
KDC_DATA:       EQU 50H     ; 8279 data port
KDC_CMD:        EQU 51H     ; 8279 command port

UART_CMD1:      EQU 60H     ; 8256
UART_CMD2:      EQU 61H     ; 8256
UART_CMD3:      EQU 62H     ; 8256
UART_MODE:      EQU 63H     ; 8256
UART_PORT1C:    EQU 64H     ; 8256
UART_INTEN:     EQU 65H     ; 8256
UART_INTAD:     EQU 66H     ; 8256
UART_BUFFER:    EQU 67H     ; 8256
UART_PORT1:     EQU 68H     ; 8256
UART_PORT2:     EQU 69H     ; 8256
UART_TIMER1:    EQU 6AH     ; 8256

SOUND_PORT:     EQU 72H     ; Sound output port



WRITE_DISPLAY:  EQU 80H
CLEAR_DISPLAY:  EQU 0C1H
END_INT:        EQU 0E0H
READ_FIFO:      EQU 40H

; Note definitions (D0-D3)
NOTE_C8:        EQU 00H
NOTE_C9:        EQU 01H
NOTE_B8:        EQU 02H
NOTE_A8S:       EQU 03H
NOTE_A8:        EQU 04H
NOTE_G8S:       EQU 05H
NOTE_G8:        EQU 06H
NOTE_F8S:       EQU 07H
NOTE_F8:        EQU 08H
NOTE_E8:        EQU 09H
NOTE_D8S:       EQU 0AH
NOTE_D8:        EQU 0BH
NOTE_C8S:       EQU 0CH

; Length definitions (D4-D5)
LENGTH_SHORT:   EQU 00H     ; Short note (00 << 4 = 00H)
LENGTH_MEDIUM:  EQU 10H     ; Medium note (01 << 4 = 10H)
LENGTH_LONG:    EQU 20H     ; Long note (10 << 4 = 20H)
LENGTH_EXTRA:   EQU 30H     ; Extra long note (11 << 4 = 30H)

; Octave definitions (D6-D7)
OCTAVE_LOW:     EQU 00H     ; Low octave (00 << 6 = 00H)
OCTAVE_MID:     EQU 40H     ; Mid octave (01 << 6 = 40H)
OCTAVE_HIGH:    EQU 80H     ; High octave (10 << 6 = 80H)
OCTAVE_EXTRA:   EQU 0C0H    ; Extra high octave (11 << 6 = 0C0H)

; Delay constants
DELAY_COUNT:    EQU 0CFFFH   ; Delay counter for note duration

; Initialize KDC
KDCINI: MVI     A, 0CH      ; DD MODE 1 KKK MODE 4
        OUT     KDC_CMD
        MVI     A, 3EH      ; CLOCK 102 KHZ
        OUT     KDC_CMD
        MVI     A, CLEAR_DISPLAY
        OUT     KDC_CMD
        MVI     A, END_INT
        OUT     KDC_CMD
        IN      KDC_CMD
        RET

; Initialize 8256 UART
UARTINI: MVI    A, 01H
        OUT     UART_CMD1
        MVI     A, 15H
        OUT     UART_CMD2
        MVI     A, 0E1H
        OUT     UART_CMD3
        MVI     A, 03H
        OUT     UART_MODE
        MVI     A, 70H
        OUT     UART_PORT1C
        MVI     A, 30H
        OUT     UART_PORT1
        MVI     A, 0FFH
        OUT     UART_PORT2
        RET

; Play note function
; Input: A = note data (D0-D3=note, D4-D5=length, D6-D7=octave)
PLAY_NOTE: PUSH B
           PUSH D
           PUSH H
           
           ; Display note on LEDs for visual feedback
           MOV    B, A      ; Save note data
           MVI    C, 0      ; Display on first lamp
           CALL   WRLAMP
           
           ; Send note to sound port
           MOV    A, B
           OUT    SOUND_PORT
           
           ; Wait for note duration
           CALL   DELAY
           
           ; Clear display
           MVI    B, 00H
           CALL   WRLAMP
           
           POP    H
           POP    D
           POP    B
           RET

; Delay function for note timing
DELAY:    PUSH B
          PUSH D
          LXI    B, DELAY_COUNT
DELAYLP:  DCX    B
          MOV    A, B
          ORA    C
          JNZ    DELAYLP
          POP    D
          POP    B
          RET

; Write to lamps function
WRLAMP:   MOV    A, C
          ORI    WRITE_DISPLAY
          OUT    KDC_CMD
          MOV    A, B
          OUT    KDC_DATA
          RET

; Write to display function  
WR_DIG:   MOV    A, C
          ADI    8
          ORI    WRITE_DISPLAY
          OUT    KDC_CMD
          MOV    A, B
          RLC
          RLC
          RLC
          RLC
          ORA    D
          OUT    KDC_DATA
          RET

; Test sequence functions
PLAY_SCALE: PUSH B
            ; Play C major scale
            MVI    A, NOTE_C8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_E8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_F8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_F8S | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_F8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_E8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_C8 | OCTAVE_LOW | LENGTH_MEDIUM
            CALL   PLAY_NOTE
            MVI    A, NOTE_B8 | OCTAVE_MID | LENGTH_LONG
            CALL   PLAY_NOTE
            POP    B
            RET

PLAY_CHORD:  PUSH B
             ; Play C major chord (C-E-G)
             MVI    A, NOTE_C8 | OCTAVE_MID | LENGTH_LONG
             CALL   PLAY_NOTE
             MVI    A, NOTE_E8 | OCTAVE_MID | LENGTH_LONG
             CALL   PLAY_NOTE
             MVI    A, NOTE_G8 | OCTAVE_MID | LENGTH_LONG
             CALL   PLAY_NOTE
             POP    B
             RET

PLAY_DEMO:   PUSH B
             ; Demonstrate different octaves with same note
             MVI    A, NOTE_C8 | OCTAVE_LOW | LENGTH_SHORT
             CALL   PLAY_NOTE
             MVI    A, NOTE_C8 | OCTAVE_MID | LENGTH_SHORT
             CALL   PLAY_NOTE
             MVI    A, NOTE_C8 | OCTAVE_HIGH | LENGTH_SHORT
             CALL   PLAY_NOTE
             MVI    A, NOTE_C8 | OCTAVE_EXTRA | LENGTH_SHORT
             CALL   PLAY_NOTE
             POP    B
             RET

PLAY_LENGTHS: PUSH B
              ; Demonstrate different lengths with same note
              MVI    A, NOTE_A8 | OCTAVE_MID | LENGTH_SHORT
              CALL   PLAY_NOTE
              MVI    A, NOTE_A8 | OCTAVE_MID | LENGTH_MEDIUM
              CALL   PLAY_NOTE
              MVI    A, NOTE_A8 | OCTAVE_MID | LENGTH_LONG
              CALL   PLAY_NOTE
              MVI    A, NOTE_A8 | OCTAVE_MID | LENGTH_EXTRA
              CALL   PLAY_NOTE
              POP    B
              RET

; Main program
reset:   CALL   KDCINI
         CALL   UARTINI

start:   ; Initialize display
         MVI    B, 0FFH    ; Turn on all lamps
         MVI    C, 0
LAMPLC:  CALL   WRLAMP
         INR    C
         MOV    A, C
         CPI    8
         JNZ    LAMPLC
         
         ; Initialize digits
         MVI    B, 03H
         MVI    D, 04H
         MVI    C, 0
DIGILC:  CALL   WR_DIG
         INR    C
         MOV    A, C
         CPI    8
         JNZ    DIGILC

         ; Play test sequences in a loop
MAINLP:  MVI    A, END_INT
         OUT    KDC_CMD

         ; Display "PLAY" pattern on LEDs
         MVI    B, 55H     ; Binary pattern for display
         MVI    C, 0
         CALL   WRLAMP
         
         ; Play different test sequences
         CALL   PLAY_SCALE
         CALL   DELAY
         CALL   PLAY_CHORD
         CALL   DELAY
         CALL   PLAY_DEMO
         CALL   DELAY
         CALL   PLAY_LENGTHS
         CALL   DELAY
         
         ; Brief pause between cycles
         LXI    B, 0FFFFH
PAUSELP: DCX    B
         MOV    A, B
         ORA    C
         JNZ    PAUSELP
         
         JMP    MAINLP
         HLT