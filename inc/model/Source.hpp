#pragma once
#include <vector>
#ifndef OPENANIMECLI_SOURCE_HPP
#define OPENANIMECLI_SOURCE_HPP

#include "Common.hpp"
#include "model/Fansub.hpp"
#include <string>

class SourceFile
{
public:
    std::string file;
    int resolution;

    static SourceFile from_json(const JSON &json, const std::string &base_path)
    {
        SourceFile sourceFile;

        sourceFile.file = base_path + json["file"].get<std::string>();
        sourceFile.resolution = json["resolution"];

        return sourceFile;
    }
};

class Source
{
public:
    std::vector<Fansub> fansubs;
    Fansub fansub;
    std::vector<SourceFile> files;
    std::string skip_times;
    bool has_next;
    bool has_prev;

    static Source
    from_json(const JSON &json, const std::string cdn_link, const std::string &slug, int season)
    {
        Source source;

        for (const auto &fansub_json : json["fansubs"]) {
            source.fansubs.push_back(Fansub::from_json(fansub_json));
        }
        for (const auto &file_json : json["episodeData"]["files"]) {
            source.files.push_back(SourceFile::from_json(
                file_json, cdn_link + slug + "/" + std::to_string(season) + "/"));
        }
        source.fansub = Fansub::from_json(json["episodeData"]["fansub"]);
        source.skip_times =
            json["episodeData"].contains("skipTimes") ? json["episodeData"]["skipTimes"] : "";
        source.has_next = json["episodeData"]["hasNextEpisode"];
        source.has_prev = json["episodeData"]["hasPrevEpisode"];

        return source;
    }
};

#endif // OPENANIMECLI_SOURCE_HPP
