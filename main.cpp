#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "src/include/graphics.h"
#include "src/include/cpu.h"

const int
    WIDTH = 800,
    HEIGHT = 600;

int main() {
    Mem mem;
    CPU cpu;
    
    mem.LoadFile("os.out");
    cpu.Reset( mem );

    InitWindow( WIDTH, HEIGHT, "6502 Emulator" );

    while ( !WindowShouldClose() ) {
        cpu.Execute( mem, false );

        if ( GetCharPressed() ) {
            char key = W_ProcessKey();
            if (key) {
                mem[0xD010] = key;
                mem[0xD011] = 1;
            }
        }

        if ( mem[0xD013] ) { // If draw call
            BeginDrawing();
                ClearBackground(BLACK);
                W_RenderTextBuffer();
            EndDrawing();
        }
    }



    return 0;
}