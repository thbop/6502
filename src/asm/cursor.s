.import ECHOXY, WAITARROW

cursor_x = $C010
cursor_y = $C011

.export cursor_run
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
    rts
cursor_x_dec:
    dec cursor_x
    rts
cursor_y_inc:
    inc cursor_y
    rts
cursor_y_dec:
    dec cursor_y
    rts

cursor_draw:
    ldx cursor_x
    ldy cursor_y
    jsr ECHOXY
    rts