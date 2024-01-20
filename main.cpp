#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <SDL2/SDL.h>

#include <ascii.h>
#include <graphics.h>
#include <cpu.h>



int main( int argc, char *argv[] ) {
    Mem mem;
    CPU cpu;
    

    mem.LoadFile("thbop.out");
    cpu.Reset( mem );

    // mem.PrintRegion( 0x8000, 0x9000 );

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

    char key;

    if ( W_Init() ) {
        // Create window
        window = W_CreateWindow("6502", WIDTH, HEIGHT);
        if ( window != NULL ) {
            renderer = W_CreateRenderer( window );
            SDL_Event event;
            bool running = true;

            while ( running ) {
                // Check events
                while ( SDL_PollEvent( &event ) ) {
                    switch( event.type ) {
                        case SDL_QUIT:
                            running = false;
                            break;
                        case SDL_KEYUP:
                            key = W_ProcessKey( event.key.keysym );
                            if (key) {
                                mem[0xD010] = key;
                                mem[0xD011] = 1; // KBDCR
                            }
                            break;
                        default:
                            break;
                        
                    }
                }

                // Clear screen
                W_ClearScreen(renderer, W_LR_PALETTE[0]);


                // Colors
                // for (int i = 0; i < 40; i++) {
                //     for (int j = 0; j < 48; j++) {
                //         W_Draw40( renderer, i, j, (i + j) % 15 );
                //     }
                // }
                
                cpu.Execute( mem, false );
                
                if (mem[0xD013]) {
                    // Text rendering
                    W_RenderTextBuffer( renderer );

                    // Update screen
                    SDL_RenderPresent( renderer );
                }
            }
        }
    }

    W_Close( window, renderer );

    return EXIT_SUCCESS;

}