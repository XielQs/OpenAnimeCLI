#include "ArgumentParser.hpp"
#include "Common.hpp"
#include <iostream>
#include <string>

ArgumentParser::ArgumentParser(int argc, char *argv[]) : use_fzf(true), argc(argc), argv(argv) {}

void ArgumentParser::parse()
{
    bool hit_command = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            displayHelp();
            exit(0);
        } else if (arg == "--version") {
            displayVersion();
            exit(0);
        } else if (arg == "--fzf") {
            use_fzf = true;
            hit_command = true;
        } else if (arg == "--no-fzf") {
            use_fzf = false;
            hit_command = true;
        } else if (!hit_command && arg.compare(0, 2, "--") != 0) {
            anime_name += arg + " ";
            continue;
        } else {
            std::cerr << "Bilinmeyen secenek: " << arg << std::endl;
            displayHelp();
            exit(1);
        }
    }

    if (!anime_name.empty())
        anime_name = anime_name.substr(0, anime_name.size() - 1); // remove trailing space
}

void ArgumentParser::displayHelp() const
{
    std::cout << "Kullanim: openanime-cli [anime adi] [secenekler]" << std::endl;
    std::cout << "Secenekler:" << std::endl;
    std::cout << "  --help       Bu yardimi goster" << std::endl;
    std::cout << "  --version    Versiyon bilgisini goster" << std::endl;
    std::cout << "  --fzf        FZF ile secim yap" << std::endl;
}

void ArgumentParser::displayVersion() const
{
    std::cout << "openanime-cli versiyon " << CLI_VERSION << std::endl;
    std::cout << "Copyright (C) 2025 XielQ ve OpenAnime Labs." << std::endl;
    std::cout
        << "openanime-cli bir topluluk projesidir, OpenAnime Labs ile HICBIR baglantisi yoktur."
        << std::endl;
}
