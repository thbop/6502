.import ECHO, ECHOXY, WAITARROW
.import cursor_run

init:
    ldy #00
    ldx #00
    jsr draw_v_line
    jmp loop


draw_v_line:
    


loop:
    jsr cursor_run
    jmp loop
