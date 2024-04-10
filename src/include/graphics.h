#ifndef GRAPHICS_H
#define GRAPHICS_H

// CHANGE THESE COLORS TO MATCH OTHER IMG
const Color W_LR_PALETTE[16] = { // 40x40 or 40x48
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

const Color  W_HR_PALETTE[8] = { // 280x192
    {0, 0, 0, 255},
    {18, 202, 7, 255},
    {202, 19, 254, 255},
    {190, 190, 190, 255},
    {0, 0, 0, 255},
    {240, 92, 0, 255},
    {36, 151, 255, 255},
    {255, 255, 255, 255},
};



char W_ProcessKey() { // This function is really bad atm
    char character = GetKeyPressed();
    // printf( "%x\n", character );
    if ( !IsKeyDown(KEY_LEFT_SHIFT) ) {
        if ( character > 0x40 && character < 0x5E ) {
            character += 0x20;
        }
    }
    return character;
}



// Specific drawing functions, graphics memory / processing stuff
// https://en.wikipedia.org/wiki/Apple_II_graphics
// 40x40 or 40x48
// 280x192



// 52x28
char
    W_TextBuffer[28][44],
    W_TextPointer[2] = {0, 0};



void W_ClearTextBuffer() {
    for (int j = 0; j < 28; j++) {
        for (int i = 0; i < 44; i++) {
            W_TextBuffer[j][i] = 0;
        }
    }
}

void W_RenderTextBuffer() {
    for (int j = 0; j < 28; j++) {
        DrawText( W_TextBuffer[j], 5, 20*j+j*2, 20, W_HR_PALETTE[1] );
    }
}


#endif