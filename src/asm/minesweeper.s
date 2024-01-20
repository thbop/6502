.import ECHO, ECHOXY, WAITARROW
; .import cursor_run

draw_x = $A010
draw_y = $A011

cursor_x = $C010
cursor_y = $C011

init:
    lda #01
    sta cursor_x
    sta cursor_y
    jsr draw_v_lines
    jmp loop

draw_v_lines:
    jsr draw_v_line
    inx
    inx
    cpx #16
    bne draw_v_lines
    rts

draw_v_line:
    lda #'|'
    jsr ECHOXY
    inc draw_y
    ldy draw_y
    cpy #16
    bne draw_v_line
    ldy #00
    sty draw_y
    rts


cursor_run:
    lda #' '
    jsr cursor_draw
    jsr WAITARROW
    jsr cursor_process_input
    lda #'0'
    jsr cursor_draw
    rts

; 4f RIGHT
; 50 LEFT
; 52 UP
; 51 DOWN
cursor_process_input:
    cmp #$4F
    beq cursor_x_inc
    cmp #$50
    beq cursor_x_dec
    cmp #$52
    beq cursor_y_dec
    cmp #$51
    beq cursor_y_inc
    rts

cursor_x_inc:
    inc cursor_x
    inc cursor_x
    rts
cursor_x_dec:
    dec cursor_x
    dec cursor_x
    rts
cursor_y_inc:
    inc cursor_y
    inc cursor_y
    rts
cursor_y_dec:
    dec cursor_y
    dec cursor_y
    rts

cursor_draw:
    ldx cursor_x
    ldy cursor_y
    jsr ECHOXY
    rts

loop:
    jsr cursor_run
    jmp loop
