                                ;
IN               = $0200        ; Input buffer to 027F
KBD              = $D010        ; Keyboard stuff
DSP              = $D012        ; Display

                LDA #$11
                PHA
                PHA

                LDA #'H'
                JSR ECHO
                RTS

ECHO:           STA DSP
                RTS

