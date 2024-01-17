#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include <ascii.h>
#include <graphics.h>
#include <cpu.h>



int main( int argc, char *argv[] ) {
    Mem mem;
    CPU cpu;
    cpu.Reset( mem );

    mem.LoadFile("thbop.out");


    // cpu.Execute( 50, mem, true );
    // cpu.printRegFlags();


    // mem.PrintRegion( 0x1F0, 0x1FF );

    const int WIDTH = 800, HEIGHT = 600;


    // char txt[] = "int main() {\n   printf(\"Hello World!\");\n}";
    // for (int i = 0; i < sizeof(txt); i++) {
    //     W_PushText( txt[i] );
    // }

    // Define the SDL window and screen surface
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if ( W_Init() ) {
        // Create window
        window = W_CreateWindow("6502", WIDTH, HEIGHT);
        if ( window != NULL ) {
            renderer = W_CreateRenderer( window );
                SDL_Event windowEvent;
            bool running = true;

            while ( running ) {
                if ( SDL_PollEvent( &windowEvent ) ) {
                    if ( SDL_QUIT == windowEvent.type ) { running = false; }
                }
                // Clear screen
                W_ClearScreen(renderer, W_LR_PALETTE[0]);


                // Colors
                // for (int i = 0; i < 40; i++) {
                //     for (int j = 0; j < 48; j++) {
                //         W_Draw40( renderer, i, j, (i + j) % 15 );
                //     }
                // }

                cpu.Execute( 50, mem, false ); // Still a cycles issue

                // Text rendering
                W_RenderTextBuffer( renderer );

                // Update screen
                SDL_RenderPresent( renderer );
            }
        }
    }

    W_Close( window, renderer );

    return EXIT_SUCCESS;

}