#pragma once

#include <string>

#include "../cache/LRUCache.hpp"
#include "../http/RestClient.hpp"
#include "../patterns/CircuitBreaker.hpp"

class ClientHandler
{
public:
  ClientHandler(int clientFd,
                LRUCache<std::string, std::string> &cache,
                RestClient &restClient,
                CircuitBreaker &circuitBreaker);

  void operator()();

private:
  int clientFd_;
  LRUCache<std::string, std::string> &cache_;
  RestClient &restClient_;
  CircuitBreaker &circuitBreaker_;

  bool sendLine(const std::string &line);
  void handleLine(const std::string &line);
};
