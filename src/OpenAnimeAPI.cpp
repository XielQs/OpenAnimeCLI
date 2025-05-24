#include "OpenAnimeAPI.hpp"
#include "Helpers.hpp"
#include "model/Anime.hpp"
#include "model/AnimeSearch.hpp"
#include "model/Source.hpp"
#include <format>
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
    checkResponse(response);

    auto json_response = safeParse(response.text);
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
}

Anime OpenAnimeAPI::fetchAnime(const std::string &slug)
{
    cpr::Response response = client.get("/anime/" + slug);
    checkResponse(response);

    auto item = safeParse(response.text);

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

        .episode_runtime = item["episodeRuntime"].is_null() ? 0 : item.value("episodeRuntime", 0),
        .number_of_episodes =
            item["numberOfEpisodes"].is_null() ? 0 : item.value("numberOfEpisodes", 0),
        .number_of_seasons =
            item["numberOfSeasons"].is_null() ? 0 : item.value("numberOfSeasons", 0),

        .seasons = seasons,
    });

    return anime;
}

Source OpenAnimeAPI::fetchSource(const std::string &slug,
                                 int season,
                                 int episode,
                                 const std::string &fansub_id)
{
    cpr::Response response =
        client.get(std::format("/anime/{}/season/{}/episode/{}", slug, season, episode),
                   {{"fansub_id", fansub_id}});
    checkResponse(response);

    std::string cdn_link = fetchCDNLink(slug, season, episode);

    auto json_response = safeParse(response.text);
    Source sources = Source::from_json(json_response, cdn_link, slug, season);

    if (sources.files.empty())
        throw std::runtime_error("No sources found for the given anime");

    return sources;
}

std::string OpenAnimeAPI::fetchCDNLink(const std::string &slug, int season, int episode) const
{
    cpr::Response response =
        client.get(std::format("https://openani.me/anime/{}/{}/{}", slug, season, episode));
    checkResponse(response);

    try {
        std::regex regex_pattern(R"(CDN_LINK:"(.*?)\",)");
        std::smatch match;
        std::regex_search(response.text, match, regex_pattern);
        if (match.empty())
            throw std::runtime_error("CDN link not found");

        return match[1].str();
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to extract CDN link: " + std::string(e.what()));
    }
}

void OpenAnimeAPI::checkResponse(const cpr::Response &response) const
{
    if (response.status_code != 200) {
        std::string body =
            response.text.empty()
                ? "No response body"
                : (isJSON(response.text) ? JSON::parse(response.text).dump(4) : response.text);

        if (isJSON(response.text)) {
            auto json_response = safeParse(response.text);
            if (json_response.contains("error") && json_response["error"].is_string()) {
                body = json_response["error"];
            }
        }

        if (body == "Episode not found") {
            std::cerr << "Sectiginiz bolum bulunamadi, bunun nedeni animenin daha yapim asamasinda "
                         "olmasi olabilir"
                      << std::endl;
            exit(1);
        }

        throw std::runtime_error(
            std::format("Request failed with status code {}: {}", response.status_code, body));
    }
}
