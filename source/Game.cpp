#include <iostream>
#include <ogc/mutex.h>
#include <ogc/lwp_watchdog.h>
#include <stdio.h>
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
    this->current_subround = 0;
    this->num_battles_needed = 0;
    this->rounds = {
        {
            .num_drawing_rounds = 3,
            .draw_timeout = 10000,
            .write_timeout = INT32_MAX,
            .shirt_timeout = 10000,
            .battle_timeout = 10000,
            .wait_timeout = 5000,
        }
    };
    this->image_list.clear();
    this->slogan_list.clear();
    this->submitted_players.clear();
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
            new_next_state = Shirt;
            break;
        case Shirt:
            new_next_state = Battle;
            break;
        case Battle:
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

    this->current_timeout = ticks_to_millisecs(gettime());

    switch (this->current_state)
    {
        case Wait:
            this->current_timeout += this->rounds.at(current_round).wait_timeout;
            break;
        case Lobby:
            this->current_timeout += INT32_MAX;
            break;
        case Draw:
            // TODO: ADD EXTRA 5 seconds of time for auto submits at the end of drawing.
            this->current_timeout += this->rounds.at(current_round).draw_timeout + 5000;
            this->submitted_players.clear();
            break;
        case Write:
            this->current_timeout += this->rounds.at(current_round).write_timeout;
            this->submitted_players.clear();
            break;
        case Shirt:
            this->current_timeout += this->rounds.at(current_round).shirt_timeout; 
            // TODO: Create shirt offerings here    
            break;
        case Battle:
            this->current_timeout += this->rounds.at(current_round).battle_timeout;
            // TODO: Create battles here
            // TODO: Battle state crashes server
            this->num_battles_needed = 1;
            this->current_subround = 0;
            break;
        case End:
            this->current_timeout += INT32_MAX;
            break;
    }
}

uint32_t Game::check_next_state_timeout()
{
    int64_t time_left = current_timeout - ticks_to_millisecs(gettime());
    // std::cout << "Time left: " << time_left << std::endl;
    if (time_left < 0)
    {
        this->go_to_next_state();
        // std::cout << "STATE chAnge" << std::endl;
        return 0;
    }
    else
    {
        return time_left;
    }
        
}

// TODO: save images to sd card using libfat?
void Game::add_image_from_blob(uint32_t creator, char* blob, size_t blob_size)
{
    char buf[50];
    sprintf(buf, "sd://data/wiiko/tmp/%X%X.jpg", creator, image_list.size());
    FILE* image_file = fopen(buf, "wb");
    if (image_file == NULL)
    {
        std::cout << "could not open " << buf << std::endl;
        return;
    }

    fwrite(blob, sizeof(char), blob_size, image_file);

    fclose(image_file);

    this->image_list.emplace_back(creator, buf);
    std::cout << creator << " submitted. Path: " << buf << std::endl;
    this->submitted_players[creator]++;
    return;
}

void Game::add_slogan(uint32_t creator, std::string slogan)
{
    this->image_list.emplace_back(creator, slogan);
    std::cout << creator << " submitted" << std::endl;
    this->submitted_players[creator]++;
    return;
}

uint8_t Game::player_num_submissions(uint32_t id)
{
    return this->submitted_players[id];
}