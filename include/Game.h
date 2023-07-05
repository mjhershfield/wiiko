#pragma once
#include <ogc/mutex.h>
#include <string>
#include <vector>
#include <map>
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
        uint64_t draw_timeout;
        uint64_t write_timeout;
        uint64_t shirt_timeout;
        uint64_t battle_timeout;
        uint64_t wait_timeout;
    };

    struct PlayerSubmission {
        uint32_t creator;
        std::string data;

        PlayerSubmission(uint32_t creator, std::string data): creator(creator), data(data) {};
    };

    GameState current_state;
    GameState next_state;
    uint64_t current_timeout;
    std::vector<RoundSettings> rounds;
    uint8_t current_round;
    uint8_t current_subround;
    uint8_t num_battles_needed;
    std::vector<PlayerSubmission> image_list;
    std::vector<PlayerSubmission> slogan_list;
    std::map<uint32_t, uint8_t> submitted_players;

public:
    Game();
    ~Game();

    GameState get_current_state();
    void go_to_next_state();
    uint32_t check_next_state_timeout();
    void reset();

    void add_image_from_blob(uint32_t creator, char* blob, size_t blob_size);
    void add_slogan(uint32_t creator, std::string slogan);
    uint8_t player_num_submissions(uint32_t id);
};