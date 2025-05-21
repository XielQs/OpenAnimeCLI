#pragma once
#ifndef OPENANIMECLI_ARGUMENTPARSER_HPP
#define OPENANIMECLI_ARGUMENTPARSER_HPP

#include <string>

class ArgumentParser
{
public:
    ArgumentParser(int argc, char *argv[]);

    void parse();
    void displayHelp() const;
    void displayVersion() const;

    bool use_fzf;
    std::string anime_name = "";

private:
    int argc;
    char **argv;
};

#endif // OPENANIMECLI_ARGUMENTPARSER_HPP
