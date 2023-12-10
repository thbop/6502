; A little test program for my custom 6502 assembler


; I{10} = 0
; J{11} = 0
; K{12} = F0{unsigned}
; loop 16ish times
;   I++
;   J++
;   K++

STA $10 ; var I
STA $11 ; var J

LDA #F0
STA $12 ; var K = F0

; loop
LDA $10
ADC #01
STA $10 ; var I

LDA $11
ADC #01
STA $11 ; var J

LDA $12
ADC #01
STA $12 ; var K

BCC #ED ; -19