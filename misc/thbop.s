                                ;
IN               = $0200        ; Input buffer to 027F
KBD              = $D010        ; Keyboard stuff
DSP              = $D012        ; Display


                LDA #'H'
                JSR ECHO

ECHO:           STA DSP
                RTS

