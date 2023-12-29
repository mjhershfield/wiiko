/*===========================================
    Wii Double Down Mii interface
    Code     : mdbrim

    miidraw.h
============================================*/
#ifndef MIIDRAW_H
#define MIIDRAW_H

#include <mii.h>
#include <grrlib.h>
#include "default_miis.h"

enum Character {
    Whale = 0,
    Horse,
    TwoFace,
    // Pumpkin,
    // Dino,
    Hamburger,
    // Shark,
    MonaLisa,
    Fish,
    Butterfly,
    Turkey,
    UpsideDown,
    Robot,
    Frog,
    Superhero,
    Dad
};

class MiiImage {
private:
    Character character;
    f32 pos_x;
    f32 pos_y;
    bool needs_updating;
    int eye_override;
    int mouth_override;
    bool blink_enabled;
    int blink_timer;
    bool blinking;
    bool bobbing_enabled;
    float bobbing_param;
    float bobbing_param_inc;
    bool show_name_card;

    GRRLIB_texImg* texture;

public:
    static void initialize(GRRLIB_ttfFont* font);

    void default_expression();
    void make_happy();
    void make_sad();
    void make_shocked();
    void make_mad();
    void set_character(Character new_character);
    void set_location(f32 x, f32 y);
    void set_blinking_enabled(bool enable);
    void set_bobbing_enabled(bool enable);
    void set_name_card_enabled(bool enable);
    void composite();
    void render();

    MiiImage(Character character, f32 pos_x, f32 pos_y);
    ~MiiImage();
};

#endif