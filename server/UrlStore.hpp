#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <mutex>

class UrlStore
{
public:
    std::string shorten(const std::string& url);

    std::optional<std::string> resolve(const std::string& code);

    bool remove(const std::string& code);

private:
    std::string generate_code();

private:
    std::unordered_map<std::string, std::string> db_;

    std::mutex mutex_;
};