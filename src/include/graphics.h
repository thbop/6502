#ifndef GRAPHICS_H
#define GRAPHICS_H

// CHANGE THESE COLORS TO MATCH OTHER IMG
int W_LR_PALETTE[16][4] = {
    {0, 0, 0, 255},
    {114, 38, 64, 255},
    {64, 51, 127, 255},
    {228, 52, 254, 255},
    {14, 89, 64, 255},
    {128, 128, 128, 255},
    {27, 154, 254, 255},
    {191, 179, 255, 255},
    {64, 76, 0, 255},
    {228, 101, 1, 255},
    {241, 166, 191, 255},
    {128, 128, 128, 255},
    {27, 203, 1, 255},
    {191, 204, 128, 255},
    {141, 217, 191, 255},
    {255, 255, 255, 255}
};

// ADD HIGH RES PALETTE
    

void W_PrintError( std::string message ) {
    std::cout << message << " SDL_Error:" << SDL_GetError() << std::endl;
}

bool W_Init() {
    bool failure = SDL_Init( SDL_INIT_EVERYTHING ) < 0;
    if ( failure ) {
        W_PrintError( "SDL could not be initialized!" );
    }
    return !failure;
}

void W_Close( SDL_Window*& window, SDL_Renderer*& renderer ) {
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    window = NULL; renderer = NULL;
    SDL_Quit();
}

SDL_Window* W_CreateWindow( const char* name, int width, int height) {
    SDL_Window* window = SDL_CreateWindow( name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_ALLOW_HIGHDPI );
        if ( window == NULL ) {
                W_PrintError( "Could not create window!" );
            }
    return window;
}

SDL_Renderer* W_CreateRenderer( SDL_Window*& window ) {
    SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if ( renderer == NULL ) {
        W_PrintError( "Could not create renderer!" );
    }
    else {
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    }
    return renderer;
}

void W_ClearScreen( SDL_Renderer*& renderer, int (&color)[4] ) {
    SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
    SDL_RenderClear( renderer );
}

void W_DrawRect( SDL_Renderer*& renderer, int (&color)[4], const SDL_Rect rect ) {
    SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
    SDL_RenderFillRect( renderer, &rect );
}

// Specific drawing functions, graphics memory / processing stuff
// https://en.wikipedia.org/wiki/Apple_II_graphics

#endif