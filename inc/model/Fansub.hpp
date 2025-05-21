#pragma once
#ifndef OPENANIMECLI_FANSUB_HPP
#define OPENANIMECLI_FANSUB_HPP

#include "Common.hpp"
#include <string>

class Fansub
{
public:
    std::string id;
    std::string name;
    std::string secure_name;

    static Fansub from_json(const JSON &json)
    {
        Fansub fansub;

        fansub.id = json["id"];
        fansub.name = json["name"];
        fansub.secure_name = json["secureName"];

        return fansub;
    }
};

#endif // OPENANIMECLI_FANSUB_HPP
