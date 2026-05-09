#include "UrlStore.hpp"

#include <random>

std::string UrlStore::generate_code()
{
    static const std::string chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    static std::random_device rd;

    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string code;

    for (int i = 0; i < 6; i++)
    {
        code += chars[dist(gen)];
    }

    return code;
}

std::string UrlStore::shorten(const std::string& url)
{
    std::scoped_lock lock(mutex_);

    std::string code;

    do
    {
        code = generate_code();
    }
    while (db_.contains(code));

    db_[code] = url;

    return code;
}

std::optional<std::string> UrlStore::resolve(const std::string& code)
{
    std::scoped_lock lock(mutex_);

    auto it = db_.find(code);

    if (it == db_.end())
    {
        return std::nullopt;
    }

    return it->second;
}

bool UrlStore::remove(const std::string& code)
{
    std::scoped_lock lock(mutex_);

    return db_.erase(code) > 0;
}