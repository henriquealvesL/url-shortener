#pragma once

#include <atomic>

#include "../cache/LRUCache.hpp"
#include "../http/RestClient.hpp"
#include "../patterns/CircuitBreaker.hpp"

class SocketServer
{
public:
  SocketServer(int port,
               LRUCache<std::string, std::string> &cache,
               RestClient &restClient,
               CircuitBreaker &circuitBreaker);

  bool start();
  void stop();

private:
  int port_;
  int serverFd_{-1};
  std::atomic<bool> running_{false};

  LRUCache<std::string, std::string> &cache_;
  RestClient &restClient_;
  CircuitBreaker &circuitBreaker_;

  bool setupSocket();
};
