#pragma once
#ifndef OPENANIMECLI_HTTPCLIENT_HPP
#define OPENANIMECLI_HTTPCLIENT_HPP

#include <cpr/cpr.h>
#include <string>

class HttpClient
{
public:
    HttpClient(std::string base_url, std::string authToken = "");

    void setToken(std::string token) { auth_token = token; }
    cpr::Response get(const std::string &path, cpr::Parameters params = {}) const;
    cpr::Response post(const std::string &path, const std::string &body = "") const;

private:
    std::string base_url;
    std::string auth_token;

    cpr::Header default_headers() const;
};

#endif // OPENANIMECLI_HTTPCLIENT_HPP
