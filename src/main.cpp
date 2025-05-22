#include "ArgumentParser.hpp"
#include "Helpers.hpp"
#include "OpenAnimeAPI.hpp"
#include "lib/inquirer.hpp"
#include "model/Anime.hpp"
#include "model/AnimeSearch.hpp"
#include "model/Fansub.hpp"
#include "model/Source.hpp"
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

    std::vector<std::string> results_list;

    for (const auto &item : results) {
        results_list.push_back(item.english + (item.isMovie() ? " (film)" : ""));
    }

    int anime_index = selectPrompt("Bir anime secin", results_list, parser.use_fzf, inquirer);

    return results[anime_index];
}

Fansub selectFansub(const std::vector<Fansub> &fansubs)
{
    std::vector<std::string> fansub_list;
    for (const auto &fansub : fansubs) {
        fansub_list.push_back(fansub.name);
    }

    int selected_index = selectPrompt("Bir fansub secin", fansub_list, parser.use_fzf, inquirer);

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

    if (source_file.file.empty()) {
        std::cout << "Video oynatilamadi" << std::endl;
        exit(1);
    }

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
    if (system(command.c_str()) != 0) {
        std::cout << "Video oynatilamadi" << std::endl;
        exit(1);
    }
}

void controlScreen(const Anime &anime,
                   const long union_,
                   const std::vector<SourceFile> &sources,
                   const std::vector<Fansub> &fansubs,
                   const Fansub &fansub,
                   const SourceFile &source_file)
{
    int episode = (union_ >> (sizeof(int) * 8)) & 0xFFFFFFFF;
    int season = union_ & 0xFFFFFFFF;

    std::vector<std::string> controls;
    int season_episode_count = !anime.isMovie() ? anime.seasons.at(season - 1).episode_count : 0;

    if (!anime.isMovie() && (episode < season_episode_count || season < anime.number_of_seasons)) {
        controls.push_back((episode == season_episode_count) ? "sonraki sezon" : "sonraki bölüm");
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
        (!anime.isMovie() ? "Sezon " + std::to_string(season) + " Bölüm " + std::to_string(episode)
                          : "Film") +
        " - " + std::to_string(source_file.resolution) + "p oynatılıyor";

    while (true) {
        int selected_index = selectPrompt(title, controls, parser.use_fzf, inquirer);
        const std::string &selected = controls[selected_index];

        // TODO: implement the logic for each control
        if (selected == "cikis") {
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
