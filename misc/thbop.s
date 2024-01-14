                                ;
IN               = $0200        ; Input buffer to 027F
KBD              = $D010        ; Keyboard stuff
KBDCR            = $D011
DSP              = $D012        ; Display
DSPCR            = $D013        ; Decides when a char is set.

                LDA #$11        ; Some random place to go to after execution
                PHA
                PHA

                LDA #'H'
                JSR ECHO
                RTS

ECHO:           STA DSP
                LDA #01
                STA DSPCR       ; My odd way to detect a character change
                LDA #00
                STA DSPCR
                RTS

