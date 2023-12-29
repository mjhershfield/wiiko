#ifndef TIMER_H
#define TIMER_H

#include <grrlib.h>

class Timer {
private:
    int seconds_remaining;
    int frame_counter;
    bool running;

    float center_x;
    float center_y;
public:
    static void initialize(GRRLIB_ttfFont* font);

    void set_time(int seconds);
    void start_timer(int seconds);
    void pause_timer();
    bool is_running();
    void set_center(float x, float y);
    void tick();
    void render();
};

#endif