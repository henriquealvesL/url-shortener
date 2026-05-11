#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "cache/LRUCache.hpp"
#include "http/RestClient.hpp"
#include "patterns/CircuitBreaker.hpp"
#include "socket/SocketServer.hpp"

int main()
{
  const int proxyPort = 9000;
  const std::string restBaseUrl = "http://127.0.0.1:8080";
  const size_t cacheCapacity = 5;

  LRUCache<std::string, std::string> cache(cacheCapacity);
  RestClient restClient(restBaseUrl);
  CircuitBreaker circuitBreaker(3, 2, std::chrono::seconds(10));
  SocketServer server(proxyPort, cache, restClient, circuitBreaker);

  std::cout << "Proxy TCP iniciado na porta " << proxyPort << "\n";
  std::cout << "Servidor REST alvo: " << restBaseUrl << "\n";
  std::cout << "Capacidade do cache LRU: " << cacheCapacity << "\n";

  if (!server.start())
  {
    std::cerr << "Nao foi possivel iniciar o proxy\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
