    org 0                               ; set the program origin

OutputAddress equ 0x8000                ; Memory mapped address of the LED array
StackAddress equ 0x0200                 ; Use stack between 0x100 and 0x200

StartValue equ %10110111                ; default value that is displayed on LED

InterruptValue1 equ %10101010           ; value this is displayed on LED for interrupt 1
InterruptValue2 equ %01010101           ; value this is displayed on LED for interrupt 2

;
; Entry Point
;
; Initialises the stack, calls the subroutine 'multiply_numbers', and then exits
start:
    LD SP, StackAddress                 ; Load SP with stack address
    IM 0                                ; Set interrupt mode 0 (simulate 8080)
    EI                                  ; Enable Interrupts
    LD A, StartValue                    ; Display the default value on LED Array
    CALL display_A

loop:
    JP loop                             ; infinite loop

interrupt_1:
    DEFS 0x0020 - $                     ; Fill up to address 0x0020 with zeros
    org 0x0020                          ; RST 0x20
    LD A, InterruptValue1
    CALL display_A
    RETI

interrupt_2:
    DEFS 0x0028 - $                     ; Fill up to address 0x0028 with zeros
    org 0x0028                          ; RST 0x28
    LD A, InterruptValue2
    CALL display_A
    RETI

;
; display_A
;
; Write the value in A to the LED array
; NOTE: Modifies HL
display_A:
    LD HL, OutputAddress                ; initialize HL with the output address
    LD (HL), A                          ; write the value in A to the address stored in HL
    RET
