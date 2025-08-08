# BreadboardZ80
Building a breadboard computer around a Z80 CPU

# Assembly Code

The 'asm' directory contains assembly code that can be assembled and run on the z80.

You need to install the sjasmplus assembler from github.

> https://github.com/z00m128/sjasmplus/

Use sjasmplus to compile your chosen assembly program into a binary.

> sjasmplus --raw=program.bin asm/YourProgram.asm 

# Embed machine code in Arduino Program 

Convert the binary into a c-style hexcode array that can be included into the Arduino program.

> xxd -i program.bin

Paste that into BreadboardZ80.ino at the position marked with comments.

# Write machine code to EEPROM

> minipro -p "AT28C256" -w program.bin

Write the program to th EEPROM

> minipro -p "AT28C256" -m program.bin

Verify that the program has been written to the EEPROM

> minipro -p "AT28C256" -r eeprom.bin

Read data from EEPROM to eeprom.bin.
