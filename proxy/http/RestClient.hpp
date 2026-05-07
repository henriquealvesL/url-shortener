#pragma once

#include <string>

class RestClient
{
public:
  struct ShortenResult
  {
    bool ok{false};
    std::string code;
    std::string shortUrl;
    std::string error;
  };

  struct ResolveResult
  {
    bool ok{false};
    std::string originalUrl;
    std::string error;
  };

  struct RemoveResult
  {
    bool ok{false};
    std::string error;
  };

  explicit RestClient(std::string baseUrl);

  ShortenResult shorten(const std::string &url) const;
  ResolveResult resolve(const std::string &code) const;
  RemoveResult remove(const std::string &code) const;

private:
  std::string baseUrl_;
};
