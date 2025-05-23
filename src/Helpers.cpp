#include "Common.hpp"
#include "lib/inquirer.hpp"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <unistd.h>
#include <vector>

std::string execute(const std::string &cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    FILE *raw_pipe = popen(cmd.c_str(), "r");
    if (!raw_pipe) {
        throw std::runtime_error("popen() failed!");
    }

    std::unique_ptr<FILE, decltype(&pclose)> pipe(raw_pipe, pclose);

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

std::string randomString(int length)
{
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static thread_local std::mt19937 rng{std::random_device{}()};
    static thread_local std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i)
        result += charset[dist(rng)];

    return result;
}

int selectPrompt(const std::string question,
                 const std::vector<std::string> &options,
                 bool use_fzf,
                 alx::Inquirer inquirer)
{
    std::string selected;

    if (use_fzf) {
        std::string tmp_file = std::filesystem::temp_directory_path() / randomString(16);
        std::ofstream file(tmp_file);
        if (!file)
            throw std::runtime_error("Failed to open temporary file");

        for (const auto &option : options) {
            file << option << "\n";
        }

        file.close();
        std::string command = "fzf --reverse --cycle --prompt='" + question + "' < " + tmp_file;
        selected = execute(command);
        std::filesystem::remove(tmp_file);
    } else {
        system("clear");
        selected = inquirer.add_question({"select", question, options}).ask();
    }

    if (selected.empty()) {
        std::cerr << "No selection made." << std::endl;
        exit(1);
    }

    if (selected.back() == '\n')
        selected.pop_back();

    auto selection = find(options.begin(), options.end(), selected);
    if (selection == options.end()) {
        std::cerr << "Selection not found." << std::endl;
        exit(1);
    }

    return distance(options.begin(), selection);
}

JSON safeParse(const std::string &text)
{
    try {
        return JSON::parse(text);
    } catch (const JSON::parse_error &e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }
}
