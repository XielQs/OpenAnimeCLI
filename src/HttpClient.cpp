#include "HttpClient.hpp"
#include "cpr/parameters.h"

HttpClient::HttpClient(std::string base_url, std::string auth_token)
    : base_url(std::move(base_url)), auth_token(std::move(auth_token))
{
}

cpr::Response HttpClient::get(const std::string &path, cpr::Parameters params) const
{
    cpr::Session session;
    // check if the path is absolute or relative
    session.SetUrl(cpr::Url{path.compare(0, 4, "http") != 0 ? (base_url + path) : path});
    session.SetHeader(default_headers());
    session.SetParameters(std::move(params));

    return session.Get();
}

cpr::Response HttpClient::post(const std::string &path, const std::string &body) const
{
    cpr::Session session;
    session.SetUrl(cpr::Url{base_url + path});
    session.SetHeader(default_headers());
    session.SetBody(cpr::Body{body});

    return session.Post();
}

cpr::Header HttpClient::default_headers() const
{
    cpr::Header headers = {
        {"Content-Type", "application/json"},
        {"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:138.0) Gecko/20100101 Firefox/138.0"},
    };

    if (!auth_token.empty()) {
        headers["Authorization"] = auth_token;
    }

    return headers;
}
