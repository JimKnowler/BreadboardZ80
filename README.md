# BreadboardZ80
Building a breadboard computer around a Z80 CPU

# Assembly Code

The 'asm' directory contains assembly code that can be assembled and run on the z80.

You need to install the sjasmplus assembler from github.

> 

Use sjasmplus to compile your chosen assembly program into a binary.

> sjasmplus --raw=program.bin asm/YourProgram.asm 

Convert the binary into a c-style hexcode array that can be included into the Arduino program.

> xxd -i program.bin

Paste that into BreadboardZ80.ino at the position marked with comments.
