#include <grrlib.h>

#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <mii.h>
#include "MiiImage.h"
#include "Timer.h"
#include "Shirt.h"

#include "shirt_png.h"
#include "drawing_png.h"
#include "AzeretMonoBold_ttf.h"

void drawMiiBadge(GRRLIB_texImg* mii_tex, f32 centerx, f32 centery);
void buildShirt(GRRLIB_texImg* base_shirt, GRRLIB_texImg* drawing, u32 bg_color, const char* text, GRRLIB_ttfFont* font, GRRLIB_texImg* final_shirt);
void drawButton(f32 x, f32 y);

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    WPAD_Init();

    GRRLIB_texImg* base_shirt_tex = GRRLIB_LoadTexture(shirt_png);

    GRRLIB_ttfFont *azeret_font = GRRLIB_LoadTTF(AzeretMonoBold_ttf, AzeretMonoBold_ttf_size);

    MiiImage::initialize(azeret_font);
    Timer::initialize(azeret_font);
    Shirt::initialize(azeret_font);

    GRRLIB_ClipDrawing(0, 0, rmode->fbWidth, rmode->efbHeight);
    GRRLIB_SetAntiAliasing(true);

    // Prep Miis and shirts (static textures)
    MiiImage mii1(Character::MonaLisa, 90, 400);
    mii1.set_bobbing_enabled(true);
    mii1.set_name_card_enabled(false);
    MiiImage mii2(Character::Hamburger, 550, 80);
    mii2.set_bobbing_enabled(true);
    mii2.set_name_card_enabled(true);

    Timer timer;
    timer.set_center(320, 240);
    timer.start_timer(10);

    // Max 10 characters per line on shirt.

    Shirt shirt1(160, 230, -10, GRRLIB_LoadTexture(drawing_png), 0xFF0000FF, "shirt 1");
    shirt1.set_bobbing_enabled(true);
    Shirt shirt2(480, 250, 10, GRRLIB_LoadTexture(drawing_png), 0xFF0000FF, "shirt 2!!!");
    shirt2.set_bobbing_enabled(true);

    // Loop forever
    while(1) {

        WPAD_ScanPads();  // Scan the Wiimotes

        // If [HOME] was pressed on the first Wiimote, break out of the loop
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  break;

        // ---------------------------------------------------------------------
        // Place your drawing code here
        // ---------------------------------------------------------------------
        if (!timer.is_running()) {
            mii1.set_character((Character) (rand() % Character::Dad));
            mii2.set_character((Character) (rand() % Character::Dad));
            timer.start_timer(10);
        }
        mii1.composite();
        mii2.composite();
        shirt1.composite();
        shirt2.composite();

        GRRLIB_FillScreen(0x3B3D91FF);    // Clear the screen
        GRRLIB_PrintfTTF(170, 10, azeret_font, "FIGHT!", 80, 0xFFFFFFFF);

        shirt1.render();
        mii1.render();

        shirt2.render();
        mii2.render();

        timer.tick();
        timer.render();

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_FreeTexture(base_shirt_tex);
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
