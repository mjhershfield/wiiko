#include <iostream>
#include <ogc/mutex.h>
#include <ogc/lwp_watchdog.h>
#include "Game.h"

Game::Game()
{
    LWP_MutexInit(&this->mutex, false);
    this->reset();
}

void Game::reset()
{
    this->current_state = Lobby;
    this->next_state = Wait;
    this->current_timeout = INT32_MAX;
    this->current_round = 0;
    this->rounds = {
        {
            .num_drawing_rounds = 3,
            .draw_timeout = 90000,
            .write_timeout = 60000,
            .shirt_timeout = 60000,
            .battle_timeout = 20000,
            .wait_timeout = 5000,
        }
    };
    settime(0);
}

Game::~Game()
{
    LWP_MutexDestroy(this->mutex);
}

Game::GameState Game::get_current_state()
{
    return this->current_state;
}

void Game::go_to_next_state()
{
    GameState new_next_state;
    switch (this->current_state)
    {
        case Wait:
            new_next_state = Wait;
            break;
        case Lobby:
            new_next_state = Draw;
            this->current_subround = 0;
            break;
        case Draw:
            if (++(this->current_subround) < this->rounds[current_round].num_drawing_rounds)
                new_next_state = Draw;
            else
                new_next_state = Write;
            break;
        case Write:
            // TODO: Create shirt offerings here
            new_next_state = Shirt;
            break;
        case Shirt:
            new_next_state = Battle;
            // TODO: Create battles here
            this->num_battles_needed = 1;
            this->current_subround = 0;
            break;
        case Battle:
            // TODO: we crash when finding the next state for battle?
            if (++(this->current_subround) < this->num_battles_needed)
                new_next_state = Battle;
            else if (++this->current_round < this->rounds.size())
                new_next_state = Draw;
            else
                new_next_state = End;
            break;
        case End:
            new_next_state = End;
            break;
        default:
            new_next_state = Wait;
            std::cout << "UNEXPECTED CURRENT STATE. NEXT STATE IS WAIT" << std::endl;
            break;
    }
    this->current_state = this->next_state;
    this->next_state = new_next_state;

    switch (this->current_state)
    {
        case Wait:
            this->current_timeout = this->rounds.at(current_round).wait_timeout;
            break;
        case Lobby:
            this->current_timeout = INT32_MAX;
            break;
        case Draw:
            this->current_timeout = this->rounds.at(current_round).draw_timeout;
            break;
        case Write:
            this->current_timeout = this->rounds.at(current_round).write_timeout;
            break;
        case Shirt:
            this->current_timeout = this->rounds.at(current_round).shirt_timeout; 
            break;
        case Battle:
            this->current_timeout = this->rounds.at(current_round).battle_timeout;
            break;
        case End:
            this->current_timeout = INT32_MAX;
            break;
    }

    settime(0);
}

void Game::check_next_state_timeout()
{
    if (diff_msec(0, gettime()) >= this->current_timeout)
        this->go_to_next_state();
}