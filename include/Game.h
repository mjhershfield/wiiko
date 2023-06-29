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
    GameState current_state;
    GameState next_state;
    uint32_t timeout;

    Game();
    ~Game();

    uint32_t add_player(std::string username);
};