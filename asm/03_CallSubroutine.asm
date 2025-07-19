    org 0                               ; set the program origin
                                        ; NOTE: 'org' is not a label, so it can't 
                                        ;       be at the start of the line
                                        ;       (i.e. it must be indented)

ValueX equ 15                           ; Constants start with labels, so they must 
ValueY equ 12                           ; start at the beginning of the line
OutputAddress equ 0x8000                ; Memory mapped address of the LED array
StackAddress equ 0x0200                 ; Use stack between 0x100 and 0x200
;
; Entry Point
;
; Initialises the stack, calls the subroutine 'multiply_numbers', and then exits
start:
    LD SP, StackAddress
    CALL multiply_numbers               ; Call the subroutine
    HALT                                ; halt the CPU

;
; multiply_numbers
;
; Multiply ValueX and ValueY, and then display them on the LED array
;
multiply_numbers:
    LD A, 0                             ; initialize A to zero 
    LD B, ValueX                        ; initialize B with the value of X

loop:
    ADD A, ValueY                       ; add Y to A
    DEC B                               ; decrement B by 1

    JP Z, loop_finished                 ; if B is 0, then finish the loop
    JP loop                             ; otherwise, jump back to loop

loop_finished:
    CALL display_result                  ; call subroutine
    RET                                 ; return from this subroutine
;
; display_result
;
; Write the value in A to the LED array
; Modifies HL
display_result:
    LD HL, OutputAddress                ; initialize HL with the output address
    LD (HL), A                          ; write the value in A to the address stored in HL
    RET
