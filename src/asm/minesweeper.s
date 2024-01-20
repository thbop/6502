.import ECHO, ECHOXY, WAITARROW
.import cursor_run

init:
    lda #'|'
    ; jsr draw_v_line
    jmp loop


draw_v_line:
    jsr ECHOXY
    iny
    cpy #30
    bne draw_v_line
    ldy #00
    rts


loop:
    jsr cursor_run
    jmp loop
