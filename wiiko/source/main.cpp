#include <grrlib.h>

#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <mii.h>
#include "miidraw.h"
#include "mii_config.h"

#include "shirt_png.h"
#include "drawing_png.h"
#include "AzeretMonoBold_ttf.h"

void drawMiiBadge(GRRLIB_texImg* mii_tex, f32 centerx, f32 centery);
void buildShirt(GRRLIB_texImg* base_shirt, GRRLIB_texImg* drawing, u32 bg_color, const char* text, GRRLIB_ttfFont* font, GRRLIB_texImg* final_shirt);

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    WPAD_Init();

    GRRLIB_texImg* base_shirt_tex = GRRLIB_LoadTexture(shirt_png);
    // 128x128 drawings
    GRRLIB_texImg* drawing_tex = GRRLIB_LoadTexture(drawing_png);
    GRRLIB_texImg* shirt1_tex = GRRLIB_CreateEmptyTexture(base_shirt_tex->w, base_shirt_tex->h);
    GRRLIB_SetMidHandle(shirt1_tex, true);

    GRRLIB_ttfFont *quicksand_font = GRRLIB_LoadTTF(AzeretMonoBold_ttf, AzeretMonoBold_ttf_size);

    GRRLIB_MiisInit();

    GRRLIB_texImg* mii_tex1 = GRRLIB_CreateEmptyTexture(180, 200);
    GRRLIB_SetMidHandle(mii_tex1, true);

    GRRLIB_texImg* mii_tex2 = GRRLIB_CreateEmptyTexture(180, 200);
    GRRLIB_SetMidHandle(mii_tex2, true);

    GRRLIB_ClipDrawing(0, 0, rmode->fbWidth, rmode->efbHeight);
    GRRLIB_SetAntiAliasing(true);

    // Prep Miis and shirts (static textures)
    GRRLIB_BuildMii(miis[0], 0, 0, 0, 0, mii_tex1);
    GRRLIB_BuildMii(miis[1], 0, 0, 0, 0, mii_tex2);

    // Max 10 characters per line on shirt.
    buildShirt(base_shirt_tex, drawing_tex, 0xFF0000FF, "Text here!", quicksand_font, shirt1_tex);

    // Loop forever
    while(1) {

        WPAD_ScanPads();  // Scan the Wiimotes

        // If [HOME] was pressed on the first Wiimote, break out of the loop
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;

        // ---------------------------------------------------------------------
        // Place your drawing code here
        // ---------------------------------------------------------------------
        GRRLIB_FillScreen(0x3B3D91FF);    // Clear the screen
        GRRLIB_PrintfTTF(170, 10, quicksand_font, "FIGHT!", 80, 0xFFFFFFFF);

        GRRLIB_DrawImg(160, 230, shirt1_tex, -10, 1, 1, 0xFFFFFFFF);
        drawMiiBadge(mii_tex1, 90, 400);

        GRRLIB_DrawImg(480, 250, shirt1_tex, 10, 1, 1, 0xFFFFFFFF);
        drawMiiBadge(mii_tex2, 550, 80);

        GRRLIB_Circle(320, 240, 35, 0x000000FF, true);
        GRRLIB_Circle(320, 240, 30, 0xFFFFFFFF, true);
        GRRLIB_PrintfTTF(295, 215, quicksand_font, "60", 40, 0x000000FF);

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_FreeTexture(base_shirt_tex);
    GRRLIB_FreeTexture(shirt1_tex);
    GRRLIB_FreeTexture(mii_tex1);
    GRRLIB_FreeTexture(mii_tex2);
    GRRLIB_FreeTTF(quicksand_font);
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}

void drawMiiBadge(GRRLIB_texImg* mii_tex, f32 centerx, f32 centery)
{
    GRRLIB_Circle(centerx, centery, 65, 0x000000FF, true);
    GRRLIB_Circle(centerx, centery, 60, 0xFFFFFFFF, true);
    GRRLIB_DrawImg(centerx, centery+10, mii_tex, 0, 1, 1, 0xFFFFFFFF);
}

void buildShirt(GRRLIB_texImg* base_shirt, GRRLIB_texImg* drawing, u32 bg_color, const char* text, GRRLIB_ttfFont* font, GRRLIB_texImg* final_shirt)
{
    GRRLIB_CompoStart();
    GRRLIB_DrawImg(0, 0, base_shirt, 0, 1, 1, bg_color);
    GRRLIB_DrawImg(80, 26, drawing, 0, 1, 1, 0xFFFFFFFF);
    GRRLIB_PrintfTTF(80, 154, font, text, 20, 0xFFFFFFFF);
    GRRLIB_CompoEnd(0, 0, final_shirt);
}
