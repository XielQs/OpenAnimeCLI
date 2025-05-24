#pragma once
#ifndef OPENANIMECLI_HELPERS_HPP
#define OPENANIMECLI_HELPERS_HPP

#include "Common.hpp"
#include "lib/inquirer.hpp"
#include <string>

std::string execute(const std::string &cmd);

std::string randomString(int length);

bool isCommand(const std::string &cmd);

int selectPrompt(const std::string question,
                 const std::vector<std::string> &options,
                 bool use_fzf,
                 alx::Inquirer inquirer);

JSON safeParse(const std::string &text);

#endif // OPENANIMECLI_HELPERS_HPP
