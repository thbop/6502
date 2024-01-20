.import ECHO, ECHOXY, WAITARROW
.import cursor_run

draw_x = $A010
draw_y = $A011

init:
    jsr draw_v_lines
    jmp loop

draw_v_lines:
    jsr draw_v_line
    inx
    inx
    cpx #32
    bne draw_v_lines
    rts

draw_v_line:
    lda #'|'
    jsr ECHOXY
    inc draw_y
    ldy draw_y
    cpy #20
    bne draw_v_line
    ldy #00
    sty draw_y
    rts


loop:
    jsr cursor_run
    jmp loop
