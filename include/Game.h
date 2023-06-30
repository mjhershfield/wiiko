#pragma once
#include <ogc/mutex.h>
#include <string>
#include <vector>
#include "PlayerList.h"

// TODO: be consistent with what should be public and what should be private
// TODO: add message queue for shutdown notification?

class Game
{
public:
    enum GameState
    {
        Wait,
        Lobby,
        Draw,
        Write,
        Shirt,
        Battle,
        End
    };

    mutex_t mutex;
    PlayerList players;

private:

    struct RoundSettings {
        uint8_t num_drawing_rounds;
        uint32_t draw_timeout;
        uint32_t write_timeout;
        uint32_t shirt_timeout;
        uint32_t battle_timeout;
        uint32_t wait_timeout;
    };

    GameState current_state;
    GameState next_state;
    uint32_t current_timeout;
    std::vector<RoundSettings> rounds;
    uint8_t current_round;
    uint8_t current_subround;
    uint8_t num_battles_needed;

public:
    Game();
    ~Game();

    GameState get_current_state();
    void go_to_next_state();
    void check_next_state_timeout();
    void reset();
};