                                ;
IN               = $0200        ; Input buffer to 027F
KBD              = $D010        ; Keyboard stuff
KBDCR            = $D011
DSP              = $D012        ; Display
DSPCR            = $D013        ; Decides when a char is set.


MAIN:           LDA #'H'
                JSR ECHO
                LDA #'e'
                JSR ECHO
                LDA #'l'
                JSR ECHO
                LDA #'l'
                JSR ECHO
                LDA #'o'
                JSR ECHO
                LDA #' '
                JSR ECHO
                LDA #'W'
                JSR ECHO
                LDA #'o'
                JSR ECHO
                LDA #'r'
                JSR ECHO
                LDA #'l'
                JSR ECHO
                LDA #'d'
                JSR ECHO
                LDA #'!'
                JSR ECHO
                
                JMP WAIT

RESET:          JSR MAIN

ECHO:           STA DSP
                LDA #01
                STA DSPCR       ; My odd way to detect a character change
                STY DSPCR
                RTS

WAIT:           LDA #80
                BMI WAIT

.segment "VECTORS"
                .WORD $0000
                .WORD RESET
                .WORD RESET
