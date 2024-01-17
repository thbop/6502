                                ;
IN              = $0200         ; Input buffer to 027F
KBD             = $D010         ; Keyboard stuff
KBDCR           = $D011

DSP             = $D012         ; Display
DSPCR           = $D013         ; Decides when a char is set.
cursor_ptr_x    = $D014
cursor_ptr_y    = $D015


HELLOWORLD:     LDA #'H'        ; Crude way to ECHO "Hello World!"
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

                RTS
                

RESET:          JSR HELLOWORLD  ; This needs to be changed greatly
                LDA #01
                JMP WAIT

ECHO:           STA DSP
                LDA #01
                STA DSPCR       ; My odd way to detect a character change
                STY DSPCR
                RTS

WAIT:           AND KBDCR
                BEQ WAIT        ; If 0, wait
                JMP REGKEY

REGKEY:         LDA KBD         ; ECHO character
                JSR ECHO
                JMP WAIT        ; Later add this to input buffer (IN)

.segment "VECTORS"
                .WORD $0000     ; non-maskable interrupt handler
                .WORD RESET     ; power on reset
                .WORD $0000     ; BRK/interrupt request handler
