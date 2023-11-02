#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <vector>

class PlayerList
{
public:
    enum ProfilePicture
        {
            Jesus,
            Whale,
            Horse,
            TwoFace,
            Pumpkin,
            Dino,
            Hamburger,
            Shark,
            MonaLisa,
            Panda,
            Fish,
            Frog,
            Butterfly,
            Basketball,
            Owl,
            Spaghetti,
            Turkey,
            Dragon,
            UpsideDown,
            NUM_PFPS
        };

    struct Player 
    {
        std::string username;
        std::string victory_quote;
        ProfilePicture pfp;
        bool admin;

        Player(std::string username, ProfilePicture pfp);
    };

private:
    std::map<uint32_t, Player> players;

public:
    const std::map<uint32_t, Player>& get_player_list();
    uint32_t add_player(std::string username);
    bool is_player(uint32_t id);
    bool is_admin(uint32_t id);
    const std::set<ProfilePicture>& get_taken_pfps();
    ProfilePicture get_pfp(uint32_t id);
    void set_pfp(uint32_t id, ProfilePicture pfp);
    void set_quote(uint32_t id, std::string quote);
    size_t num_players();
};