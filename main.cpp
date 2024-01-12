#include <iostream> // <- try to remove this sometime
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <SDL2/SDL.h>

#include <graphics.h>
#include <cpu.h>



int main( int argc, char *argv[] ) {
    Mem mem;
    CPU cpu;
    Printer printer;
    cpu.Reset( mem );

    mem.LoadFile("test.bin");
    cpu.Execute( 500, mem, printer, true );
    cpu.printRegFlags();

    const int WIDTH = 800, HEIGHT = 600;

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

                // Update screen
                SDL_RenderPresent( renderer );
            }
        }
    }

    W_Close( window, renderer );

    return EXIT_SUCCESS;

}