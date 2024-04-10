@echo off
cd src/os
cc65 test.c
cl65 -C bios.cfg test.s -o test.out
cd ..
cd ..