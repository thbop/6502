; Fibonacci Sequence
; By Thbop

; START pseudocode

; 1, 1, 2, 3, 5, 8, 13
; A{10} = 0
; B{11} = 1
; I{12} = 0

; loop
;   I = A
;   Print(A)
;   A = B
;   B += I

; END pseudocode

; WARNING: THIS CODE WAS DESIGNED FOR MY CUSTOM ASSEMBLER! IT PROBABLY WILL NOT WORK FOR AN ACTUAL ASSEMBLER!

LDA #01
STA $11 ; B = 1

; loop
LDA $10
STA $12 ; I = A
STA $FFF9 ; Print(A)

LDA $11
STA $10 ; A = B

ADC $12
STA $11 ; B += I

BCC #F0 ; loop