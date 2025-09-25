;=========================================================
; Intel 8085 ASM
;=========================================================

JMP START

KDC_COMMAND:  EQU 50H
KDC_DATA:     EQU 51H
MUART_CMD1:   EQU 60H
MUART_CMD2:   EQU 61H
MUART_CMD3:   EQU 62H
MUART_BUFFER: EQU 67H
MUART_STATUS: EQU 6FH
     
; ---- Initialize 8279 ----

START:      MVI  A, 0CH        ; Clear command
            OUT  KDC_COMMAND
            MVI  A, 3EH        ; Keyboard/Display mode (example: 8-char, encoded scan)
            OUT  KDC_COMMAND
            MVI  A, 0C1H       ; Enable display + keyboard
            OUT  KDC_COMMAND

; ---- Initialize 8256 ----
            MVI  A, 01H        ; Reset / enable
            OUT  MUART_CMD1
            MVI  A, 15H        ; Mode/baud setup
            OUT  MUART_CMD2
            MVI  A, 0E1H       ; Enable Tx, Rx
            OUT  MUART_CMD3

	        MVI  C, 40H        ; C holds mask for ANA C

;=========================================================
; Main Loop: Poll keypad & send status over serial
;=========================================================

MAIN_LOOP:  IN   KDC_DATA      ; Read keycode/status from 8279 FIFO
            MOV  B, A          ; Save in B

; ---- Wait until USART Tx ready ----
WAIT_TX:    IN   MUART_STATUS
            ANA  C          ; bit 6 = Transmit Buffer Empty
            JZ   WAIT_TX

; ---- Send key status ----
            MOV  A, B
            OUT  MUART_BUFFER

            JMP  MAIN_LOOP

;=========================================================
; END
;=========================================================
            HLT