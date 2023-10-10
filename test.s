; A little test program for my custom 6502 assembler

LDA #01 ; 2
ASL ; 2
BCC #FE ; 3
LDA #64 ; A check if we're done