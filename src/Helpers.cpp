#include "lib/inquirer.hpp"
#include <array>
#include <cstdlib>
#include <string>
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

int selectPrompt(std::string type,
                 std::string question,
                 const std::vector<std::string> &options,
                 bool use_fzf,
                 alx::Inquirer inquirer)
{
    std::string selected;

    if (use_fzf) {
        std::string command = "echo -e '";
        for (const auto &option : options) {
            command += option + "\n";
        }
        command.pop_back();
        command += "' | fzf --reverse --cycle --prompt='" + question + "'";
        selected = execute(command);
    } else {
        system("clear");
        selected = inquirer.add_question({type, question, options}).ask();
    }

    if (selected.empty()) {
        std::cerr << "No selection made." << std::endl;
        exit(1);
    }

    auto selection = find(options.begin(), options.end(), selected);
    if (selection == options.end()) {
        std::cerr << "Selection not found." << std::endl;
        exit(1);
    }

    return distance(options.begin(), selection);
}
