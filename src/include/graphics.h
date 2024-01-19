#ifndef GRAPHICS_H
#define GRAPHICS_H

// CHANGE THESE COLORS TO MATCH OTHER IMG
int W_LR_PALETTE[16][4] = { // 40x40 or 40x48
    {0, 0, 0, 255},
    {153, 3, 95, 255},
    {66, 4, 225, 255},
    {202, 19, 254, 255},
    {0, 115, 16, 255},
    {127, 127, 127, 255},
    {36, 151, 255, 255},
    {170, 162, 255, 255},
    {79, 81, 1, 255},
    {240, 92, 0, 255},
    {190, 190, 190, 255},
    {255, 133, 255, 255},
    {18, 202, 7, 255},
    {206, 212, 19, 255},
    {81, 245, 149, 255},
    {255, 255, 254, 255}
};

int W_HR_PALETTE[8][4] = { // 280x192
    {0, 0, 0, 255},
    {18, 202, 7, 255},
    {202, 19, 254, 255},
    {190, 190, 190, 255},
    {0, 0, 0, 255},
    {240, 92, 0, 255},
    {36, 151, 255, 255},
    {255, 255, 255, 255},
};

int W_BLACK[4] = {0, 0, 0, 255};
int W_WHITE[4] = {255, 255, 255, 255};



void W_PrintError( std::string message ) {
    printf("%s SDL_Error:%s\n", message.c_str(), SDL_GetError());
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

char W_ProcessKey( SDL_Keysym& keysym ) {
    char character = keysym.sym;
    // printf( "%x\n", character );
    if ( keysym.mod == 4097 ) {
        character -= 32;
    } else if (keysym.sym > 127) {
        character = 0;
    }
    return character;
}

void W_ClearScreen( SDL_Renderer*& renderer, int (&color)[4] ) {
    SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
    SDL_RenderClear( renderer );
}

void W_DrawRect( SDL_Renderer*& renderer, const SDL_Rect rect, int (&color)[4] ) {
    SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
    SDL_RenderFillRect( renderer, &rect );
}

// Specific drawing functions, graphics memory / processing stuff
// https://en.wikipedia.org/wiki/Apple_II_graphics
// 40x40 or 40x48
// 280x192

void W_Draw40( SDL_Renderer*& renderer, int x, int y, int color ) {
    SDL_Rect rect = { x * 20, (int)(y * 12.5), 20, 13 };
    W_DrawRect( renderer, rect, W_LR_PALETTE[color] );
}

void W_DrawASCII( SDL_Renderer*& renderer, unsigned char character, int x, int y ) {
    SDL_Rect char_pix = {x*15 + x*3 + 3, y*21 + y*3 + 3, 3, 3};
    for (int j = 0; j < 7; j++) {
        for (int i = 0; i < 5; i++) {
            if (ASCII[character][j][i]) { W_DrawRect( renderer, char_pix, W_WHITE ); }
            char_pix.x += 3;
        }
        char_pix.x = x*15 + x*3 + 3;
        char_pix.y += 3;
    }
}
// 52x28
unsigned char W_TextBuffer[28][44];
unsigned char W_TextPointer[2] = {0, 0};

// void W_PushChar( char character ) {
//     if ( character != '\n' ) {
//         W_TextBuffer[W_TextPointer[1]][W_TextPointer[0]] = character;
//         if ( W_TextPointer[0] < 44 ) { W_TextPointer[0]++; }
//         else { W_TextPointer[0] = 0; W_TextPointer[1]++; }
//     }
//     else { W_TextPointer[0] = 0; W_TextPointer[1]++; }
    
// }


void W_RenderTextBuffer( SDL_Renderer*& renderer ) {
    for (int j = 0; j < 28; j++) {
        for (int i = 0; i < 44; i++) {
            if ( W_TextBuffer[j][i] != 0 ) {
                W_DrawASCII( renderer, W_TextBuffer[j][i], i, j );
            }
            
        }
    }
}


#endif