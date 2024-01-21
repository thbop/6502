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
    ldx #01
    jsr draw_h_lines
    jmp loop

; Draw grid
draw_h_lines:
    ldy draw_y
    ldx #01
    jsr draw_h_line
    inc draw_y
    inc draw_y
    ldy draw_y
    cpy #18
    bne draw_h_lines
    rts

draw_h_line:
    ldy draw_y
    lda #'_'
    jsr ECHOXY
    inx
    inx
    cpx #17
    bne draw_h_line
    rts

draw_v_lines:
    jsr draw_v_line
    inx
    inx
    cpx #18
    bne draw_v_lines
    rts

draw_v_line:
    lda #'|'
    jsr ECHOXY
    inc draw_y
    ldy draw_y
    cpy #17
    bne draw_v_line
    ldy #00
    sty draw_y
    rts

; Cursor stuff
cursor_run:
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
    lda #' '
    jsr cursor_draw
    inc cursor_x
    inc cursor_x
    rts
cursor_x_dec:
    lda #' '
    jsr cursor_draw
    dec cursor_x
    dec cursor_x
    rts
cursor_y_inc:
    lda #' '
    jsr cursor_draw
    inc cursor_y
    inc cursor_y
    rts
cursor_y_dec:
    lda #' '
    jsr cursor_draw
    dec cursor_y
    dec cursor_y
    rts

cursor_draw:
    ldx cursor_x
    ldy cursor_y
    jsr ECHOXY
    rts

; Main loop
loop:
    jsr cursor_run
    jmp loop
