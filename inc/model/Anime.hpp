#pragma once
#ifndef OPENANIMECLI_ANIME_HPP
#define OPENANIMECLI_ANIME_HPP

#include "model/AnimeSearch.hpp"
#include "model/Season.hpp"
#include <string>
#include <vector>

struct AnimeData
{
    std::string type;
    std::string english;
    std::string turkish;
    std::string slug;
    std::string romaji;
    std::string id;
    std::string summary;

    int episode_runtime = 0;
    int number_of_episodes = 0;
    int number_of_seasons = 0;
    std::vector<Season> seasons;
};

class Anime : public AnimeSearch
{
public:
    Anime(const AnimeData &data)
        : AnimeSearch({
              .type = data.type,
              .english = data.english,
              .turkish = data.turkish,
              .slug = data.slug,
              .romaji = data.romaji,
              .id = data.id,
              .summary = data.summary,
          }),
          episode_runtime(data.episode_runtime), number_of_episodes(data.number_of_episodes),
          number_of_seasons(data.seasons.size()), seasons(data.seasons)
    {
    }

    int episode_runtime;
    int number_of_episodes;
    int number_of_seasons;
    std::vector<Season> seasons;
};

#endif // OPENANIMECLI_ANIME_HPP
