#pragma once
#ifndef OPENANIMECLI_ANIMESEARCH_HPP
#define OPENANIMECLI_ANIMESEARCH_HPP

#include <string>

struct AnimeSearchData
{
    std::string type;
    std::string english;
    std::string turkish;
    std::string slug;
    std::string romaji;
    std::string id;
    std::string summary;
};

class AnimeSearch
{
public:
    std::string type;
    std::string english;
    std::string turkish;
    std::string slug;
    std::string romaji;
    std::string id;
    std::string summary;

    AnimeSearch(const AnimeSearchData &data)
        : type(data.type), english(data.english), turkish(data.turkish), slug(data.slug),
          romaji(data.romaji), id(data.id), summary(data.summary)
    {
    }

    bool isMovie() const { return type == "movie"; }
};

#endif // OPENANIMECLI_ANIMESEARCH_HPP
