#include <ogc/mutex.h>
#include <string.h>
#include <stdexcept>
#include "Game.h"

Game::Game()
{
    LWP_MutexInit(&this->mutex, false);
    this->current_state = Lobby;
    this->next_state = Wait;
}

Game::~Game()
{
    LWP_MutexDestroy(this->mutex);
}

uint32_t Game::add_player(std::string username)
{
    return this->players.add_player(username);
}