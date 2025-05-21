#pragma once
#ifndef OPENANIMECLI_OPENANIMEAPI_HPP
#define OPENANIMECLI_OPENANIMEAPI_HPP

#include "HttpClient.hpp"
#include "model/Anime.hpp"
#include "model/AnimeSearch.hpp"
#include "model/Source.hpp"
#include <string>
#include <vector>

class OpenAnimeAPI
{
public:
    OpenAnimeAPI(const std::string &auth_token = "");

    void setAuthToken(const std::string &token);
    std::vector<AnimeSearch> searchAnime(const std::string &anime_name);
    Anime fetchAnime(const std::string &slug);
    Source fetchSource(const std::string &slug,
                       int season,
                       int episode,
                       const std::string &fansub_id = "");

private:
    std::string fetchCDNLink(const std::string &slug, int season, int episode);

    HttpClient client;
    inline static const std::string base_url = "https://api.openani.me";
    std::string auth_token;
};

#endif // OPENANIMECLI_OPENANIMEAPI_HPP
