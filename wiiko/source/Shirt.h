#ifndef SHIRT_H
#define SHIRT_H

#include <grrlib.h>
#include <string>

class Shirt {
private:
    // Code currently assumes 128x128 drawings
    GRRLIB_texImg* drawing;
    u32 bg_color;
    std::string quote;
    GRRLIB_texImg* shirt_texture;
    float pos_x;
    float pos_y;
    float rotation;

    bool bobbing_enabled;
    float bobbing_param;
    bool needs_updating;
public:
    static void initialize(GRRLIB_ttfFont* font);

    void set_position(float x, float y);
    void set_rotation(float rot);
    void set_drawing(GRRLIB_texImg* drawing);
    void set_quote(std::string quote);
    void set_bobbing_enabled(bool enabled);
    void composite();
    void render();

    Shirt(float x, float y, float rot, GRRLIB_texImg* drawing, u32 bg_color, std::string quote);
    ~Shirt();
};

#endif