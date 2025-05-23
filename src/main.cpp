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

    std::cout << "[debug]: Anime adi: " << anime_name << std::endl;

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

Fansub selectFansub(const std::vector<Fansub> &fansubs, const std::string fansub_id = "")
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
                           [](const Fansub &fansub) { return fansub.name; });

            selected_index =
                selectPrompt("Bir fansub secin (" + std::to_string(fansubs.size()) + " adet)",
                             fansub_list, parser.use_fzf, inquirer);
        }
    }

    return fansubs[selected_index];
}

void selectEpisode(const Anime &anime, int &season, int &episode)
{
    if (anime.number_of_seasons == 1) {
        season = 1;
    } else {
        while (true) {
            try {
                std::string season_input =
                    inquirer
                        .add_question(
                            {"season",
                             "Sezon (1-" + std::to_string(anime.number_of_seasons) + " arasi)",
                             alx::Type::decimal})
                        .ask();
                season = stoi(season_input);

                if (season < 1 || season > anime.number_of_seasons) {
                    std::cout << "Gecersiz sezon numarasi" << std::endl;
                    continue;
                }
                break;
            } catch (const std::invalid_argument &e) {
                std::cout << "Gecersiz sezon numarasi" << std::endl;
            } catch (const std::out_of_range &e) {
                std::cout << "Gecersiz sezon numarasi" << std::endl;
            }
        }
    }

    const int episode_count = anime.seasons.at(season - 1).episode_count;
    if (episode_count == 1) {
        episode = 1;
    } else {
        while (true) {
            try {
                std::string episode_input =
                    inquirer
                        .add_question({"episode",
                                       "Bolum (1-" + std::to_string(episode_count) + " arasi)",
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

void playVideo(const SourceFile &source_file, const Anime &anime, const long union_)
{
    int episode = (union_ >> (sizeof(int) * 8)) & 0xFFFFFFFF;
    int season = union_ & 0xFFFFFFFF;

    if (source_file.file.empty())
        throw std::runtime_error("Source file is empty");

    std::string title = anime.english + " - ";
    if (anime.isMovie()) {
        title += "Film";
    } else {
        title += "Sezon " + std::to_string(season) + " bolum " + std::to_string(episode);
    }
    title += " - " + std::to_string(source_file.resolution) + "p";

    std::string args = "--force-media-title='" + title + "' '" + source_file.file + "'";
    std::string command = "mpv " + args + " > /dev/null 2>&1 &";
    // filesystem::remove(temp_path);
    // " >" + temp_path + " 2>&1 &"
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
    sources = api.fetchSource(anime.slug, season, episode, fansub.id).files;

    SourceFile *best = nullptr;

    for (auto &s : sources) {
        if (!best || s.resolution > best->resolution || s.resolution == source_file.resolution)
            best = &s;
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
    int season_episode_count = !anime.isMovie() ? anime.seasons.at(season - 1).episode_count : 0;

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

    if (sources.size() > 1) {
        controls.push_back("cozunurluk degistir");
    }

    if (fansubs.size() > 1) {
        controls.push_back("fansub degistir");
    }

    controls.push_back("cikis");

    const std::string title =
        anime.english + " - " +
        (!anime.isMovie() ? "Sezon " + std::to_string(season) + " bolum " + std::to_string(episode)
                          : "Film") +
        " - " + std::to_string(source_file.resolution) + "p oynatiliyor";

    auto updateAndPlay = [&](int new_season, int new_episode) {
        long new_union = 0;

        new_union |= (long) new_episode << sizeof(int) * 8;
        new_union |= (long) new_season;

        updateSource(anime, new_union, source_file, sources, fansub, fansubs);
        playVideo(source_file, anime, new_union);
        controlScreen(anime, new_union, sources, fansubs, fansub, source_file);
    };

    while (true) {
        int selected_index = selectPrompt(title, controls, parser.use_fzf, inquirer);
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
            std::vector<std::string> resolution_list(sources.size());

            std::transform(
                sources.begin(), sources.end(), resolution_list.begin(),
                [](const SourceFile &src) { return std::to_string(src.resolution) + "p"; });

            int resolution_index =
                selectPrompt("Bir cozunurluk secin", resolution_list, parser.use_fzf, inquirer);
            source_file = sources[resolution_index];

            updateAndPlay(season, episode);
            break;
        } else if (selected == "fansub degistir") {
            Fansub new_fansub = selectFansub(fansubs);

            if (new_fansub.id != fansub.id) {
                fansub = new_fansub;
                sources = api.fetchSource(anime.slug, season, episode, fansub.id).files;

                auto it = std::find_if(sources.begin(), sources.end(), [&](const SourceFile &src) {
                    return src.resolution == source_file.resolution;
                });

                source_file = (it != sources.end()) ? *it : sources[0];
                updateAndPlay(season, episode);
                break;
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
    if (parser.use_fzf) {
        std::cout << "[debug]: Using fzf for selection" << std::endl;
    }

    AnimeSearch selected_anime = selectAnime(parser.anime_name);

    Anime anime = api.fetchAnime(selected_anime.slug);

    std::cout << "[debug]: Anime: " << anime.english << " (" << anime.episode_runtime << ")"
              << std::endl;

    int season = 0, episode = 0;

    if (!anime.isMovie()) {
        selectEpisode(anime, season, episode);
    }

    std::cout << "[debug]: Selected season: " << season << ", episode: " << episode << std::endl;

    Source source = api.fetchSource(anime.slug, season, episode);

    std::vector<Fansub> fansubs = source.fansubs;

    if (fansubs.empty()) {
        std::cout << "No fansubs found for: " << anime.english << std::endl;
        exit(1);
    }

    std::cout << "[debug]: Found " << fansubs.size() << " fansubs" << std::endl;

    Fansub selected_fansub = selectFansub(fansubs);

    std::cout << "[debug]: Selected fansub: " << selected_fansub.name << std::endl;

    if (source.fansub.id != selected_fansub.id) {
        source = api.fetchSource(anime.slug, season, episode, selected_fansub.id);
    }

    std::cout << "[debug]: Found " << source.files.size() << " files" << std::endl;

    SourceFile highest_source_file = source.files[0];

    for (const auto &file : source.files) {
        if (file.resolution > highest_source_file.resolution) {
            highest_source_file = file;
        }
    }
    std::cout << "[debug]: Highest resolution: " << highest_source_file.resolution << std::endl;

    // larei napiyon uglum :sobsob:
    long union_ = 0;

    union_ |= (long) episode << sizeof(int) * 8;
    union_ |= (long) season;

    playVideo(highest_source_file, anime, union_);
    controlScreen(anime, union_, source.files, fansubs, selected_fansub, highest_source_file);

    return 0;
}
