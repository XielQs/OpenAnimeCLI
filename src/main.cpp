#include "ArgumentParser.hpp"
#include "Helpers.hpp"
#include "OpenAnimeAPI.hpp"
#include "lib/inquirer.hpp"
#include "model/Anime.hpp"
#include "model/AnimeSearch.hpp"
#include "model/Fansub.hpp"
#include "model/Source.hpp"
#include <algorithm>
#include <cpr/cpr.h>
#include <format>
#include <iostream>
#include <string>
#include <vector>

auto inquirer = alx::Inquirer("openanime-cli");
OpenAnimeAPI api("");
ArgumentParser parser(0, 0);

AnimeSearch selectAnime(std::string anime_name = "")
{
    if (anime_name == "")
        anime_name = inquirer.add_question({"anime_name", "Anime adi", alx::Type::text}).ask();

    std::vector<AnimeSearch> results = api.searchAnime(anime_name);
    if (results.empty()) {
        std::cout << "Anime bulunamadi" << std::endl;
        exit(1);
    }

    std::vector<std::string> anime_list(results.size());

    std::transform(results.begin(), results.end(), anime_list.begin(),
                   [](auto &item) { return item.english + (item.isMovie() ? " (film)" : ""); });

    int anime_index = selectPrompt("Bir anime secin", anime_list, parser.use_fzf, inquirer);

    return results[anime_index];
}

Fansub selectFansub(const std::vector<Fansub> &fansubs,
                    const std::string fansub_id = "",
                    const std::string &current_fansub = "")
{
    int selected_index = -1;

    if (!fansub_id.empty()) {
        for (size_t i = 0; i < fansubs.size(); ++i) {
            if (fansubs[i].id == fansub_id) {
                selected_index = static_cast<int>(i);
                break;
            }
        }
    }

    if (selected_index == -1) {
        if (fansubs.size() == 1) {
            selected_index = 0;
        } else {
            std::vector<std::string> fansub_list(fansubs.size());

            std::transform(fansubs.begin(), fansubs.end(), fansub_list.begin(),
                           [&current_fansub](const Fansub &fansub) {
                               return (fansub.id == current_fansub ? "* " : "") + fansub.name;
                           });

            selected_index = selectPrompt(std::format("Bir fansub secin ({} adet)", fansubs.size()),
                                          fansub_list, parser.use_fzf, inquirer);
        }
    }

    return fansubs[selected_index];
}

void selectEpisode(const Anime &anime, int &season, int &episode)
{
    if (anime.number_of_seasons == 1) {
        season = 0;
    } else {
        while (true) {
            try {
                std::vector<std::string> season_list(anime.number_of_seasons);
                std::transform(anime.seasons.begin(), anime.seasons.end(), season_list.begin(),
                               [](const Season &s) { return s.name; });

                season = selectPrompt("Sezon", season_list, parser.use_fzf, inquirer);
                break;
            } catch (const std::invalid_argument &e) {
                std::cout << "Gecersiz sezon numarasi" << std::endl;
            } catch (const std::out_of_range &e) {
                std::cout << "Gecersiz sezon numarasi" << std::endl;
            }
        }
    }

    const int episode_count = anime.seasons[season].episode_count;
    if (episode_count == 1) {
        episode = 1;
    } else {
        while (true) {
            try {
                std::string episode_input =
                    inquirer
                        .add_question({"episode", std::format("Bolum (1-{} arasi)", episode_count),
                                       alx::Type::decimal})
                        .ask();
                episode = stoi(episode_input);

                if (episode < 1 || episode > episode_count) {
                    std::cout << "Gecersiz bolum numarasi" << std::endl;
                    continue;
                }
                break;
            } catch (const std::invalid_argument &e) {
                std::cout << "Gecersiz bolum numarasi" << std::endl;
            } catch (const std::out_of_range &e) {
                std::cout << "Gecersiz bolum numarasi" << std::endl;
            }
        }
    }
}

std::string formatTitle(const Anime &anime, int season, int episode)
{
    std::string season_name = anime.seasons[season].name;
    season_name =
        season_name.replace(season_name.find("Season"), std::string("Season").size(), "Sezon");

    return std::format("{}: {} - {}", anime.english, season_name,
                       anime.isMovie() ? "Film" : std::format("Bolum {}", episode));
}

void playVideo(const SourceFile &source_file, const Anime &anime, const long union_)
{
    int episode = (union_ >> (sizeof(int) * 8)) & 0xFFFFFFFF;
    int season = union_ & 0xFFFFFFFF;

    if (source_file.file.empty())
        throw std::runtime_error("Source file is empty");

    std::string args = std::format("--force-media-title='{}' '{}'",
                                   formatTitle(anime, season, episode), source_file.file);

    std::string command = "mpv " + args + " > /dev/null 2>&1 &";
    if (system(command.c_str()) != 0)
        throw std::runtime_error("Failed to play video: " + command);
}

void updateSource(const Anime &anime,
                  const long union_,
                  SourceFile &source_file,
                  std::vector<SourceFile> &sources,
                  Fansub &fansub,
                  std::vector<Fansub> &fansubs)
{
    int episode = (union_ >> (sizeof(int) * 8)) & 0xFFFFFFFF;
    int season = union_ & 0xFFFFFFFF;

    fansub = selectFansub(fansubs, fansub.id);
    sources =
        api.fetchSource(anime.slug, anime.seasons[season].season_number, episode, fansub.id).files;

    SourceFile *best = nullptr;

    for (auto &s : sources) {
        if (!best || s.resolution == source_file.resolution || s.resolution > best->resolution)
            best = &s;
        if (s.resolution == source_file.resolution)
            break; // found the same resolution, no need to continue
    }
    if (best)
        source_file = *best;
}

void controlScreen(const Anime &anime,
                   const long union_,
                   std::vector<SourceFile> &sources,
                   std::vector<Fansub> &fansubs,
                   Fansub &fansub,
                   SourceFile &source_file)
{
    int episode = (union_ >> (sizeof(int) * 8)) & 0xFFFFFFFF;
    int season = union_ & 0xFFFFFFFF;

    std::vector<std::string> controls;
    int season_episode_count = !anime.isMovie() ? anime.seasons[season].episode_count : 0;

    if (!anime.isMovie() && (episode < season_episode_count || season < anime.number_of_seasons)) {
        controls.push_back((episode == season_episode_count) ? "sonraki sezon" : "sonraki bolum");
    }

    controls.push_back("bolumu tekrar oynat");

    if (!anime.isMovie()) {
        if (episode > 1 || season > 1) {
            controls.push_back("onceki bolum");
        }
        controls.push_back("bolum sec");
    }

    if (sources.size() > 1)
        controls.push_back("cozunurluk degistir");

    if (fansubs.size() > 1)
        controls.push_back("fansub degistir");

    controls.push_back("cikis");

    auto updateAndPlay = [&](int new_season, int new_episode, bool update_source = true) {
        long new_union = 0;

        new_union |= (long) new_episode << sizeof(int) * 8;
        new_union |= (long) new_season;

        if (update_source)
            updateSource(anime, new_union, source_file, sources, fansub, fansubs);
        playVideo(source_file, anime, new_union);
        controlScreen(anime, new_union, sources, fansubs, fansub, source_file);
    };

    while (true) {
        int selected_index =
            selectPrompt(std::format("{} - {}p oynatiliyor", formatTitle(anime, season, episode),
                                     source_file.resolution),
                         controls, parser.use_fzf, inquirer);
        const std::string &selected = controls[selected_index];

        if (selected == "sonraki bolum" || selected == "sonraki sezon") {
            if (episode == season_episode_count) {
                if (season < anime.number_of_seasons) {
                    updateAndPlay(++season, 1);
                } else {
                    std::cout << "Bu sezonun son bolumu" << std::endl;
                    exit(0);
                }
            } else {
                updateAndPlay(season, ++episode);
            }
            break;
        } else if (selected == "bolumu tekrar oynat") {
            playVideo(source_file, anime, union_);
        } else if (selected == "onceki bolum") {
            if (episode == 1) {
                updateAndPlay(--season, season_episode_count);
            } else {
                updateAndPlay(season, --episode);
            }
            break;
        } else if (selected == "bolum sec") {
            selectEpisode(anime, season, episode);
            updateAndPlay(season, episode);
            break;
        } else if (selected == "cozunurluk degistir") {
            std::sort(sources.begin(), sources.end(), [](const SourceFile &a, const SourceFile &b) {
                return a.resolution > b.resolution;
            });

            std::vector<std::string> resolution_list(sources.size());

            std::transform(sources.begin(), sources.end(), resolution_list.begin(),
                           [&source_file](const SourceFile &src) {
                               return (src.resolution == source_file.resolution ? "* " : "") +
                                      std::to_string(src.resolution) + "p";
                           });

            int resolution_index =
                selectPrompt("Bir cozunurluk secin", resolution_list, parser.use_fzf, inquirer);
            source_file = sources[resolution_index];

            updateAndPlay(season, episode, false);
            break;
        } else if (selected == "fansub degistir") {
            Fansub new_fansub = selectFansub(fansubs, "", fansub.id);

            if (new_fansub.id != fansub.id) {
                fansub = new_fansub;
                updateAndPlay(season, episode);
                break;
            } else {
                playVideo(source_file, anime, union_);
            }
        } else if (selected == "cikis") {
            exit(0);
        } else {
            std::cout << "Gecersiz secim" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    parser = ArgumentParser(argc, argv);
    parser.parse();

    AnimeSearch selected_anime = selectAnime(parser.anime_name);

    Anime anime = api.fetchAnime(selected_anime.slug);

    int season = 0, episode = 0;

    if (!anime.isMovie())
        selectEpisode(anime, season, episode);

    Source source = api.fetchSource(anime.slug, anime.seasons[season].season_number, episode);

    std::vector<Fansub> fansubs = source.fansubs;

    if (fansubs.empty()) {
        std::cout << "No fansubs found for: " << anime.english << std::endl;
        exit(1);
    }

    Fansub selected_fansub = selectFansub(fansubs);

    if (source.fansub.id != selected_fansub.id) {
        source = api.fetchSource(anime.slug, anime.seasons[season].season_number, episode,
                                 selected_fansub.id);
    }

    SourceFile highest_source_file = source.files[0];

    for (const auto &file : source.files) {
        if (file.resolution > highest_source_file.resolution) {
            highest_source_file = file;
        }
    }

    // larei napiyon uglum :sobsob:
    long union_ = 0;

    union_ |= (long) episode << sizeof(int) * 8;
    union_ |= (long) season;

    playVideo(highest_source_file, anime, union_);
    controlScreen(anime, union_, source.files, fansubs, selected_fansub, highest_source_file);

    return 0;
}
