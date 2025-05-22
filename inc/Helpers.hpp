#pragma once
#ifndef OPENANIMECLI_HELPERS_HPP
#define OPENANIMECLI_HELPERS_HPP

#include "lib/inquirer.hpp"
#include <string>

std::string execute(const std::string &cmd);

std::string randomString(int length);

int selectPrompt(const std::string type,
                 const std::string question,
                 const std::vector<std::string> &options,
                 bool use_fzf,
                 alx::Inquirer inquirer);

#endif // OPENANIMECLI_HELPERS_HPP
