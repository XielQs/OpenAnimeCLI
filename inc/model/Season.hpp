#pragma once
#ifndef OPENANIMECLI_SEASON_HPP
#define OPENANIMECLI_SEASON_HPP

#include "Common.hpp"
#include <string>

class Season
{
public:
    std::string id;
    std::string name;
    bool has_episode;
    int episode_count;
    int season_number;

    static Season from_json(const JSON &json)
    {
        Season season;

        season.id = json["id"];
        season.name = json["name"];
        season.has_episode = json["hasEpisode"];
        season.episode_count = json["episode_count"];
        season.season_number = json["season_number"];

        return season;
    }
};

#endif // OPENANIMECLI_SEASON_HPP
