#include "OpenAnimeAPI.hpp"
#include "Common.hpp"
#include "model/Anime.hpp"
#include "model/AnimeSearch.hpp"
#include "model/Source.hpp"
#include <iostream>
#include <regex>
#include <vector>

OpenAnimeAPI::OpenAnimeAPI(const std::string &auth_token)
    : client(base_url, auth_token), auth_token(auth_token)
{
}

void OpenAnimeAPI::setAuthToken(const std::string &token)
{
    auth_token = token;
    client.setToken(token);
}

std::vector<AnimeSearch> OpenAnimeAPI::searchAnime(const std::string &anime_name)
{
    cpr::Response response = client.get("/anime/search", {{"q", anime_name}});

    if (response.status_code != 200) {
        std::cerr << "Error: " << response.status_line << std::endl;
        exit(1);
    }

    try {
        auto json_response = JSON::parse(response.text);
        std::vector<AnimeSearch> results;

        for (const auto &item : json_response) {
            AnimeSearch animeSearch(AnimeSearchData{
                .type = item.value("type", "tv"),
                .english = item["english"],
                .turkish = item["turkish"],
                .slug = item["slug"],
                .romaji = (item.contains("romaji") ? item["romaji"] : item["english"]),
                .id = item["id"],
                .summary = item.value("summary", ""),
            });
            results.push_back(animeSearch);
        }

        return results;
    } catch (const JSON::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        exit(1);
    }
}

Anime OpenAnimeAPI::fetchAnime(const std::string &slug)
{
    cpr::Response response = client.get("/anime/" + slug);

    if (response.status_code != 200) {
        std::cerr << "Error: " << response.status_line << std::endl;
        exit(1);
    }

    try {
        auto item = JSON::parse(response.text);

        std::vector<Season> seasons;
        if (item.contains("seasons") && !item["seasons"].is_null()) {
            for (const auto &season_json : item["seasons"]) {
                seasons.emplace_back(Season::from_json(season_json));
            }
        }

        Anime anime(AnimeData{
            .type = item.value("type", "tv"),
            .english = item["english"],
            .turkish = item["turkish"],
            .slug = item["slug"],
            .romaji = (item.contains("romaji") ? item["romaji"] : item["english"]),
            .id = item["id"],
            .summary = item.value("summary", ""),

            .episode_runtime =
                item["episodeRuntime"].is_null() ? 0 : item.value("episodeRuntime", 0),
            .number_of_episodes =
                item["numberOfEpisodes"].is_null() ? 0 : item.value("numberOfEpisodes", 0),
            .number_of_seasons =
                item["numberOfSeasons"].is_null() ? 0 : item.value("numberOfSeasons", 0),

            .seasons = seasons,
        });

        return anime;
    } catch (const JSON::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        exit(1);
    }
}

Source OpenAnimeAPI::fetchSource(const std::string &slug,
                                 int season,
                                 int episode,
                                 const std::string &fansub_id)
{
    cpr::Response response = client.get("/anime/" + slug + "/season/" + std::to_string(season) +
                                            "/episode/" + std::to_string(episode),
                                        {{"fansub", fansub_id}});

    if (response.status_code != 200) {
        std::cerr << "Error: " << response.status_line << std::endl;
        exit(1);
    }

    std::string cdn_link = fetchCDNLink(slug, season, episode);

    try {
        auto json_response = JSON::parse(response.text);
        Source sources = Source::from_json(json_response, cdn_link, slug, season);

        if (sources.files.empty()) {
            std::cerr << "No sources found for this episode." << std::endl;
            exit(1);
        }

        return sources;
    } catch (const JSON::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        exit(1);
    }
}

std::string OpenAnimeAPI::fetchCDNLink(const std::string &slug, int season, int episode)
{
    cpr::Response response = client.get("https://openani.me/anime/" + slug + "/" +
                                        std::to_string(season) + "/" + std::to_string(episode));

    if (response.status_code != 200) {
        std::cerr << "Error: " << response.status_line << std::endl;
        exit(1);
    }

    try {
        std::regex regex_pattern(R"(CDN_LINK:"(.*?)\",)");
        std::smatch match;
        std::regex_search(response.text, match, regex_pattern);
        std::string cdn_link = match[1].str();
        if (cdn_link.empty()) {
            std::cerr << "CDN link not found." << std::endl;
            exit(1);
        }
        return cdn_link;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}
