    org 0                               ; set the program origin
                                        ; NOTE: 'org' is not a label, so it can't 
                                        ;       be at the start of the line
                                        ;       (i.e. it must be indented)

ValueX equ 17                           ; Constants start with labels, so they must 
ValueY equ 11                           ; start at the beginning of the line
OutputAddress equ 0x8000

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

padding:
    DEFS 0x8000 - $                     ; Pad to 32k - the size of the EEPROM