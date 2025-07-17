
ValueX equ 17
ValueY equ 11
OutputAddress equ 0x8000

;org 0  ; set the program origin


start:
    LD A, 0                             ; initialize A to zero 
    LD B, ValueX                        ; initialize B with the value of X
    LD HL, OutputAddress                ; initialize HL with the output address

loop:
    ADD A, ValueY                        ; add Y to zero
    DEC B                               ; decrement B by 1

    JP Z, write_results                 ; if B is 0, then jump to write_results
    JP loop                             ; otherwise, jump back to loop

write_results:
    LD (HL), A                          ; write the value in A to the address stored in HL
    HALT                                ; halt the CPU
