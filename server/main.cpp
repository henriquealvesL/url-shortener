#include <httplib.h>
#include <nlohmann/json.hpp>

#include <iostream>

#include "UrlStore.hpp"

using json = nlohmann::json;

int main()
{
    httplib::Server server;

    UrlStore store;

    server.Post("/shorten",
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
            response["code"] = code;

            res.set_content(response.dump() + "\n",
                            "application/json");

            res.status = 201;
        }
        catch (...)
        {
            res.status = 400;
        }
    });

    server.Get(R"(/resolve/(.+))",
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
        response["url"] = url.value();

        res.set_content(response.dump() + "\n",
                        "application/json");

        res.status = 200;
    });

    server.Delete(R"(/remove/(.+))",
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

    std::cout << "REST server running on port 8080\n";

    server.listen("0.0.0.0", 8080);
}