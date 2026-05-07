#include "RestClient.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace
{
  struct Response
  {
    bool ok{false};
    long status{0};
    std::string body;
    std::string error;
  };

  Response performRequest(const std::string &baseUrl,
                          const std::string &method,
                          const std::string &path,
                          const std::optional<std::string> &body)
  {
    Response resp;
    httplib::Client client(baseUrl);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(5, 0);
    client.set_write_timeout(5, 0);

    httplib::Result result;
    if (method == "POST")
    {
      result = client.Post(path.c_str(), body.value_or(""), "application/json");
    }
    else if (method == "GET")
    {
      result = client.Get(path.c_str());
    }
    else if (method == "DELETE")
    {
      result = client.Delete(path.c_str());
    }
    else
    {
      resp.error = "Unsupported method";
      return resp;
    }

    if (!result)
    {
      resp.error = httplib::to_string(result.error());
      return resp;
    }

    resp.status = result->status;
    resp.body = result->body;
    resp.ok = resp.status >= 200 && resp.status < 300;
    return resp;
  }
}

RestClient::RestClient(std::string baseUrl)
    : baseUrl_(std::move(baseUrl))
{
}

RestClient::ShortenResult RestClient::shorten(const std::string &url) const
{
  ShortenResult result;
  nlohmann::json payload = {{"url", url}};
  std::string body = payload.dump();
  Response resp = performRequest(baseUrl_, "POST", "/urls", body);
  if (!resp.ok)
  {
    result.error = resp.error.empty() ? "HTTP error" : resp.error;
    return result;
  }

  nlohmann::json json;
  try
  {
    json = nlohmann::json::parse(resp.body);
  }
  catch (...)
  {
    result.error = "JSON parse failed";
    return result;
  }

  result.ok = true;
  result.code = json.at("codigo").get<std::string>();
  result.shortUrl = json.at("url_curta").get<std::string>();
  return result;
}

RestClient::ResolveResult RestClient::resolve(const std::string &code) const
{
  ResolveResult result;
  Response resp = performRequest(baseUrl_, "GET", "/urls/" + code, std::nullopt);
  if (!resp.ok)
  {
    result.error = resp.status == 404 ? "Not found" : (resp.error.empty() ? "HTTP error" : resp.error);
    return result;
  }

  nlohmann::json json;
  try
  {
    json = nlohmann::json::parse(resp.body);
  }
  catch (...)
  {
    result.error = "JSON parse failed";
    return result;
  }

  result.ok = true;
  result.originalUrl = json.at("url_original").get<std::string>();
  return result;
}

RestClient::RemoveResult RestClient::remove(const std::string &code) const
{
  RemoveResult result;
  Response resp = performRequest(baseUrl_, "DELETE", "/urls/" + code, std::nullopt);
  if (!resp.ok)
  {
    result.error = resp.status == 404 ? "Not found" : (resp.error.empty() ? "HTTP error" : resp.error);
    return result;
  }

  nlohmann::json json;
  try
  {
    json = nlohmann::json::parse(resp.body);
  }
  catch (...)
  {
    result.error = "JSON parse failed";
    return result;
  }

  result.ok = true;
  return result;
}
