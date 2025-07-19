    org 0                               ; set the program origin
                                        ; NOTE: 'org' is not a label, so it can't 
                                        ;       be at the start of the line
                                        ;       (i.e. it must be indented)

OutputAddress equ 0x8000                ; Constants start with labels, so they must 
Value equ %10110100                     ; start at the beginning of the line


start:
    LD A, Value                         ; initialize A to value
    LD HL, OutputAddress                ; initialize HL with the output address

write_results:
    LD (HL), A                          ; write the value in A to the address stored in HL
    HALT                                ; halt the CPU
