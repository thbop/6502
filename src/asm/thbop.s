.import RESET, ECHO, WAIT, REGKEY


MAIN:
    LDA #'>'
    JSR ECHO

    JSR WAIT