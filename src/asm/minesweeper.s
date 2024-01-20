.import ECHO, ECHOXY, WAITARROW
.import cursor_run

loop:
    jsr cursor_run
    jmp loop
