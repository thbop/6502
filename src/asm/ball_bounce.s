.import ECHO, ECHOXY

ball_x = $10
ball_y = $11
ball_vel_x = $12
ball_vel_y = $13

MAIN:
    JSR TITLE

    LDA #01
    STA ball_x
    STA ball_y
    STA ball_vel_x
    STA ball_vel_y

    JMP LOOP

TITLE:
    LDA #'B'
    JSR ECHO
    LDA #'a'
    JSR ECHO
    LDA #'l'
    JSR ECHO
    LDA #'l'
    JSR ECHO
    RTS

BALLPHYS:
    LDA ball_x
    ADC ball_vel_x
    STA ball_x
    LDA ball_y
    ADC ball_vel_y
    STA ball_y
    RTS

DRAWBALL:
    LDA #'0'
    LDX ball_x
    LDY ball_y
    JSR ECHOXY
    RTS

REMOVEOLDBALL:
    LDA #' '
    LDY ball_y ; The Y reg is used in ECHO
    JSR ECHOXY
    RTS

LOOP:
    JSR REMOVEOLDBALL
    JSR BALLPHYS
    JSR DRAWBALL
    JMP LOOP