#include <httplib.h>
#include <nlohmann/json.hpp>

#include <iostream>

#include "UrlStore.hpp"

using json = nlohmann::json;

namespace
{
    std::string short_url_for(const httplib::Request& req, const std::string& code)
    {
        std::string host = req.get_header_value("Host");
        if (host.empty())
        {
            host = "127.0.0.1:8080";
        }

        return "http://" + host + "/" + code;
    }
}

int main()
{
    httplib::Server server;

    UrlStore store;

    server.Post("/urls",
                [&](const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            json body = json::parse(req.body);

            if (!body.contains("url"))
            {
                res.status = 400;
                return;
            }

            std::string url = body["url"];

            std::string code = store.shorten(url);

            std::cout << "[POST] shortened "
                      << url
                      << " -> "
                      << code
                      << "\n";

            json response;
            response["codigo"] = code;
            response["url_curta"] = short_url_for(req, code);

            res.set_content(response.dump() + "\n",
                            "application/json");

            res.status = 201;
        }
        catch (...)
        {
            res.status = 400;
        }
    });

    server.Get(R"(/urls/([A-Za-z0-9]{6}))",
               [&](const httplib::Request &req, httplib::Response &res)
    {
        std::string code = req.matches[1];

        auto url = store.resolve(code);

        if (!url.has_value())
        {
            res.status = 404;
            return;
        }

        std::cout << "[GET] resolved "
                  << code
                  << "\n";

        json response;
        response["url_original"] = url.value();

        res.set_content(response.dump() + "\n",
                        "application/json");

        res.status = 200;
    });

    server.Delete(R"(/urls/([A-Za-z0-9]{6}))",
                  [&](const httplib::Request &req, httplib::Response &res)
    {
        std::string code = req.matches[1];

        bool removed = store.remove(code);

        if (!removed)
        {
            res.status = 404;
            return;
        }

        std::cout << "[DELETE] removed "
                  << code
                  << "\n";

        res.status = 200;
    });

    server.Get("/health",
               [](const httplib::Request &, httplib::Response &res)
    {
        json response;
        response["status"] = "ok";

        res.set_content(response.dump() + "\n",
                        "application/json");

        res.status = 200;
    });

    server.Get(R"(/([A-Za-z0-9]{6}))",
               [&](const httplib::Request &req, httplib::Response &res)
    {
        std::string code = req.matches[1];

        auto url = store.resolve(code);

        if (!url.has_value())
        {
            res.status = 404;
            return;
        }

        std::cout << "[REDIRECT] "
                  << code
                  << " -> "
                  << url.value()
                  << "\n";

        res.set_redirect(url.value(), 302);
    });

    std::cout << "REST server running on port 8080\n";

    server.listen("0.0.0.0", 8080);
}
