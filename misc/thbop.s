.import RESET, ECHO, WAITKEY, REGKEY


MAIN:
    LDA #'>'
    JSR ECHO

    JSR WAITKEY