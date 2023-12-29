#include "Shirt.h"
#include "AzeretMonoBold_ttf.h"
#include "shirt_png.h"
#include "math.h"

static GRRLIB_texImg* base_shirt_texture = nullptr;
static GRRLIB_ttfFont* shirt_quote_font = nullptr;

void Shirt::initialize(GRRLIB_ttfFont* font)
{
    if (base_shirt_texture == nullptr && shirt_quote_font == nullptr) {
        base_shirt_texture = GRRLIB_LoadTexturePNG(shirt_png);
        shirt_quote_font = font;
    }
}

void Shirt::set_position(float x, float y)
{
    this->pos_x = x;
    this->pos_y = y;
}

void Shirt::set_rotation(float rot)
{
    this->rotation = rot;
}

void Shirt::set_drawing(GRRLIB_texImg *drawing)
{
    GRRLIB_FreeTexture(this->drawing);
    this->drawing = drawing;
    this->needs_updating = true;
}

void Shirt::set_quote(std::string quote)
{
    this->quote = quote;
    this->needs_updating = true;
}

void Shirt::set_bobbing_enabled(bool enabled)
{
    this->bobbing_enabled = enabled;
}

void Shirt::composite()
{
    if (!this->needs_updating)
        return;

    GRRLIB_CompoStart();
    GRRLIB_DrawImg(0, 0, base_shirt_texture, 0, 1, 1, this->bg_color);
    GRRLIB_DrawImg(80, 26, drawing, 0, 1, 1, 0xFFFFFFFF);
    GRRLIB_PrintfTTF(80, 154, shirt_quote_font, this->quote.c_str(), 20, 0xFFFFFFFF);
    GRRLIB_CompoEnd(0, 0, this->shirt_texture);

    this->needs_updating = false;
}

void Shirt::render()
{
    float y_to_render = this->pos_y;

    if (this->bobbing_enabled) {
        bobbing_param += 0.02;
        y_to_render += 20 * sin(this->bobbing_param);
    }

    GRRLIB_DrawImg(this->pos_x, y_to_render, this->shirt_texture, this->rotation, 1, 1, 0xFFFFFFFF);
}

Shirt::Shirt(float x, float y, float rot, GRRLIB_texImg *drawing, u32 bg_color, std::string quote)
{
    this->pos_x = x;
    this->pos_y = y;
    this->rotation = rot;
    this->drawing = drawing;
    this->quote = quote;
    this->bg_color = bg_color;
    this->bobbing_enabled = false;
    this->bobbing_param = (rand() % 6280) % 1000;
    this->needs_updating = true;

    this->shirt_texture = GRRLIB_CreateEmptyTexture(base_shirt_texture->w, base_shirt_texture->h);
    GRRLIB_SetMidHandle(this->shirt_texture, true);
}

Shirt::~Shirt()
{
    GRRLIB_FreeTexture(this->drawing);
    GRRLIB_FreeTexture(this->shirt_texture);
}
