#include "SocketServer.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

#include "ClientHandler.hpp"

SocketServer::SocketServer(int port,
                           LRUCache<std::string, std::string> &cache,
                           RestClient &restClient,
                           CircuitBreaker &circuitBreaker)
    : port_(port),
      cache_(cache),
      restClient_(restClient),
      circuitBreaker_(circuitBreaker) {}

bool SocketServer::setupSocket()
{
  serverFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd_ < 0)
  {
    return false;
  }

  int opt = 1;
  if (::setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    ::close(serverFd_);
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(static_cast<uint16_t>(port_));

  if (::bind(serverFd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
  {
    ::close(serverFd_);
    return false;
  }

  if (::listen(serverFd_, 16) < 0)
  {
    ::close(serverFd_);
    return false;
  }

  return true;
}

bool SocketServer::start()
{
  if (!setupSocket())
  {
    return false;
  }

  running_ = true;

  while (running_)
  {
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    int clientFd = ::accept(serverFd_, reinterpret_cast<sockaddr *>(&clientAddr), &len);
    if (clientFd < 0)
    {
      if (!running_)
      {
        break;
      }
      continue;
    }

    std::thread(ClientHandler(clientFd, cache_, restClient_, circuitBreaker_)).detach();
  }

  return true;
}

void SocketServer::stop()
{
  running_ = false;
  if (serverFd_ >= 0)
  {
    ::close(serverFd_);
    serverFd_ = -1;
  }
}
