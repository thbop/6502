@echo off
cd src/os
cc65 os.c
cl65 -C bios.cfg os.s -o os.out
cd ..
cd ..