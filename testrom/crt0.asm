;-------------------------------------------------------------------------
        module merkur_crt0

;-------------------------------------------------------------------------
; Include zcc_opt.def to find out some info
;-------------------------------------------------------------------------
        defc    crt0 = 1
        INCLUDE "zcc_opt.def"

;-------------------------------------------------------------------------
; Some scope definitions
;-------------------------------------------------------------------------

        EXTERN    __8085_int1
        EXTERN    __8085_int3
        EXTERN    __8085_int5
        EXTERN    __8085_int7
        EXTERN    __8085_int55
        EXTERN    __8085_int65
        EXTERN    __8085_int75

        EXTERN    _main           ;main() is always external to crt0 code

        PUBLIC    __Exit         ;jp'd to by exit()
        PUBLIC    l_dcal          ;jp(hl)
        
;-------------------------------------------------------------------------
        defc    REGISTER_SP  = 0xc7f0
        defc    CRT_ORG_CODE = 0x0000
        defc    CRT_ORG_DATA = 0x8000
        defc    CRT_ORG_BSS  = 0xc000


        INCLUDE "crt/classic/crt_rules.inc"

;-------------------------------------------------------------------------
        org     CRT_ORG_CODE
rst0:   ld      sp,__register_sp
        jp      program         ;RST0 standard entry 
        defs    $08-ASMPC

rst1:   jp intr1
        defs    $10-ASMPC

rst2:   ret                     ;RST2 not used
        defs    $18-ASMPC

rst3:   jp intr3
        defs    $20-ASMPC

rst4:   ei
        ret                     ;RST4 not used
        defs    $24-ASMPC

trap:   jp rst7                 ;TRAP not used
        defs    $28-ASMPC

rst5:   jp intr5                ;uart
        defs    $2C-ASMPC

rst55:  jp intr55               ;sound
        defs    $30-ASMPC

rst6:   jp rst7                 ;RST6 not used
        defs    $34-ASMPC

rst65:  jp      intr65
        defs    $38-ASMPC

rst7:   jp intr7
        defs    $3C-ASMPC

rst75:  jp intr75

intr1:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int1
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr3:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int3
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr5:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int5
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr55:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int55
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr65:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int65
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr7:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int7
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

intr75:
        push    b
        push    d
        push    h
        push    psw        ; saves A and flags
        call    __8085_int75
        pop     psw
        pop     h
        pop     d
        pop     b
        ei
        ret

;-------------------------------------------------------------------------
program:
;        call    target_init
        call    crt0_init
        INCLUDE "crt/classic/crt_init_heap.inc"

        call    _main           ;void main(void) so no args or retval

__Exit:
        call    crt0_exit
        jp      rst0            ;restart if main should return

l_dcal:  jp     (hl)            ;Used for function pointer calls


;-------------------------------------------------------------------------
        defc    __crt_org_bss = CRT_ORG_BSS
        defc    __crt_model = 1

        INCLUDE "crt/classic/crt_runtime_selection.inc" 
        INCLUDE "crt/classic/crt_section.inc"

;-------------------------------------------------------------------------
