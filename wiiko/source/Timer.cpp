#include "Timer.h"
#include <grrlib.h>
#include <stdio.h>
#include "AzeretMonoBold_ttf.h"

#define FRAMES_PER_SECOND 60

static GRRLIB_ttfFont* timer_font = nullptr;

void Timer::initialize(GRRLIB_ttfFont* font)
{
    if (timer_font == nullptr)
        timer_font = font;
}

void Timer::set_time(int seconds)
{
    this->seconds_remaining = seconds;
}

void Timer::start_timer(int seconds)
{
    this->seconds_remaining = seconds;
    this->frame_counter = FRAMES_PER_SECOND;
    this->running = true;
}

void Timer::pause_timer()
{
    this->running = false;
}

bool Timer::is_running()
{
    return this->running;
}

void Timer::set_center(float x, float y)
{
    this->center_x = x;
    this->center_y = y;
}

void Timer::tick()
{
    if (this->running && !this->frame_counter--) {
        this->frame_counter = FRAMES_PER_SECOND;
        this->seconds_remaining--;
        this->running = this->seconds_remaining > 0;
    }
}

void Timer::render()
{
    char time_string[4];
    snprintf(time_string, sizeof(time_string), "%02u", this->seconds_remaining);

    GRRLIB_Circle(this->center_x, this->center_y, 35, 0x000000FF, true);
    GRRLIB_Circle(this->center_x, this->center_y, 30, 0xFFFFFFFF, true);
    GRRLIB_PrintfTTF(this->center_x - 25, this->center_y - 25, timer_font, time_string, 40, 0x000000FF);
}
