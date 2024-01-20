
IN = $0200 ; Input buffer to 027F
KBD = $D010 ; Keyboard stuff
KBDCR = $D011

DSP = $D012 ; Display
DSPCR = $D013 ; Decides when a char is set.
dsp_cursor_x = $D014
dsp_cursor_y = $D015

.export RESET, ECHO, ECHOXY, WAITKEY, WAITARROW
RESET:
    ; ...
    RTS
ECHO:
    STA DSP
    LDA #08
    CMP DSP
    BEQ BACKSPACE
    JSR CHARCHANGE
    INC dsp_cursor_x ; Move the cursor
    RTS

BACKSPACE:
    JSR CHARCHANGE
    DEC dsp_cursor_x
    RTS

ECHOXY:
    STA DSP
    STX dsp_cursor_x
    STY dsp_cursor_y
    JSR CHARCHANGE
    RTS

CHARCHANGE: ; My odd way to cause a character change
    LDA #01
    LDY #00
    STA DSPCR
    STY DSPCR
    RTS

WAITKEY:
    LDA #01
    CMP KBDCR
    BEQ REGKEY
    RTS

REGKEY:
    LDA #00
    STA KBDCR
    LDA KBD ; ECHO character
    JSR ECHO
    RTS



WAITARROW:
    LDA #01
    CMP KBDCR
    BEQ REGARROW
    RTS

REGARROW:
    LDA #00
    STA KBDCR
    LDA KBD
    RTS

.segment "VECTORS"
    .WORD $0000 ; non-maskable interrupt handler
    .WORD $8000 ; power on reset
    .WORD $0000 ; BRK/interrupt request handler
