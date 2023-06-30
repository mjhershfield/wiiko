#include "PlayerList.h"
#include "ogc/system.h"
#include <vector>
#include <string>
#include <stdexcept>

PlayerList::Player::Player(std::string username, ProfilePicture pfp) : 
    username(username), victory_quote(), pfp(pfp), admin(false)
{}

std::vector<const std::string*> PlayerList::get_player_list()
{
    std::vector<const std::string*> names_list;
    names_list.reserve(this->players.size());
    for (const std::pair<const uint32_t, Player>& player : this->players)
    {
        names_list.push_back(&player.second.username);
    }

    return names_list;
}

uint32_t PlayerList::add_player(std::string username)
{
    if (username.empty())
    {
        throw std::runtime_error("username string is empty");
    }

    for (const std::pair<const uint32_t, Player>& player : this->players)
    {
        if (player.second.username == username)
        {
            throw std::runtime_error("duplicated username");
        }
    }

    int32_t new_player_id;
    do
    {
        new_player_id = rand();
    } while (this->players.count(new_player_id));
    
    ProfilePicture pfp = (ProfilePicture) (rand() % NUM_PFPS);
    this->players.emplace(new_player_id, Player(username, pfp));
    if (this->players.size() == 1)
    {
        this->players.at(new_player_id).admin = true;
    }
    return new_player_id;
}

bool PlayerList::is_player(uint32_t id)
{
    if (this->players.count(id))
    {
        return true;
    }

    return false;
}

bool PlayerList::is_admin(uint32_t id)
{
    return this->players.at(id).admin;
}

void PlayerList::set_pfp(uint32_t id, ProfilePicture pfp)
{
    if (pfp >= NUM_PFPS)
    {
        throw std::runtime_error("invalid pfp index");
    }
    this->players.at(id).pfp = pfp;
}

PlayerList::ProfilePicture PlayerList::get_pfp(uint32_t id)
{
    return this->players.at(id).pfp;
}

void PlayerList::set_quote(uint32_t id, std::string quote)
{
    this->players.at(id).victory_quote = quote;
}

size_t PlayerList::num_players()
{
    return this->players.size();
}