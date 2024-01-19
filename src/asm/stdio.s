
IN = $0200 ; Input buffer to 027F
KBD = $D010 ; Keyboard stuff
KBDCR = $D011

DSP = $D012 ; Display
DSPCR = $D013 ; Decides when a char is set.
cursor_ptr_x = $D014
cursor_ptr_y = $D015

.export RESET, ECHO, WAIT, REGKEY
RESET:
    ; ...
    RTS
ECHO:
    STA DSP
    LDA #08
    CMP DSP
    BEQ BACKSPACE
    JSR CHARCHANGE
    INC cursor_ptr_x ; Move the cursor
    RTS

BACKSPACE:
    JSR CHARCHANGE
    DEC cursor_ptr_x
    RTS

CHARCHANGE: ; My odd way to detect a character change
    LDA #01
    LDY #00
    STA DSPCR
    STY DSPCR
    RTS

; ECHOXY

WAIT:
    LDA #01
    AND KBDCR
    BEQ WAIT ; If 0, wait
    STY KBDCR ; Assuming that Y=0, clears key press
    JMP REGKEY

REGKEY:
    LDA KBD ; ECHO character
    JSR ECHO
    JMP WAIT ; Later add this to input buffer (IN)

.segment "VECTORS"
    .WORD $0000 ; non-maskable interrupt handler
    .WORD $8000 ; power on reset
    .WORD $0000 ; BRK/interrupt request handler
