@echo off
g++ -Isrc/include -IC:/mingw_dev_lib/SDL2/i686-w64-mingw32/include -LC:/mingw_dev_lib/SDL2/i686-w64-mingw32/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2
main