@echo off
ca65 src/asm/stdio.s
@REM ca65 src/asm/cursor.s
ca65 src/asm/thbop.s
ld65 -C src/asm/bios.cfg src/asm/thbop.o src/asm/stdio.o -o thbop.out
@REM python utils/hexdump.py -f thbop.out --max 500