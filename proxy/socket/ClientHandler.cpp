#include "ClientHandler.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "../protocol/MessageParser.hpp"

namespace
{
  bool sendAll(int fd, const std::string &data)
  {
    size_t total = 0;
    while (total < data.size())
    {
      ssize_t sent = ::send(fd, data.data() + total, data.size() - total, 0);
      if (sent <= 0)
      {
        return false;
      }
      total += static_cast<size_t>(sent);
    }
    return true;
  }

  std::string trimLineEnding(const std::string &line)
  {
    if (!line.empty() && line.back() == '\r')
    {
      return line.substr(0, line.size() - 1);
    }
    return line;
  }

  const char *messageTypeName(MessageParser::Type type)
  {
    switch (type)
    {
    case MessageParser::Type::Shorten:
      return "ENCURTA";
    case MessageParser::Type::Resolve:
      return "RESOLVE";
    case MessageParser::Type::Remove:
      return "REMOVE";
    case MessageParser::Type::Invalid:
      return "INVALID";
    }

    return "UNKNOWN";
  }
}

ClientHandler::ClientHandler(int clientFd,
                             LRUCache<std::string, std::string> &cache,
                             RestClient &restClient,
                             CircuitBreaker &circuitBreaker)
    : clientFd_(clientFd),
      cache_(cache),
      restClient_(restClient),
      circuitBreaker_(circuitBreaker) {}

void ClientHandler::operator()()
{
  std::string buffer;
  char chunk[1024];
  while (true)
  {
    ssize_t bytes = ::recv(clientFd_, chunk, sizeof(chunk), 0);
    if (bytes <= 0)
    {
      break;
    }
    buffer.append(chunk, static_cast<size_t>(bytes));

    size_t pos = 0;
    while ((pos = buffer.find('\n')) != std::string::npos)
    {
      std::string line = buffer.substr(0, pos);
      buffer.erase(0, pos + 1);
      handleLine(trimLineEnding(line));
    }
  }

  ::close(clientFd_);
}

bool ClientHandler::sendLine(const std::string &line)
{
  return sendAll(clientFd_, line);
}

void ClientHandler::handleLine(const std::string &line)
{
  auto parsed = MessageParser::parseLine(line);
  if (!parsed.ok)
  {
    std::cout << "[PROXY] comando invalido: " << parsed.error << "\n";
    sendLine("ERR 400 Bad request\n");
    return;
  }

  const auto &message = parsed.message;
  std::cout << "[PROXY] recebido " << messageTypeName(message.type)
            << " " << message.argument << "\n";

  if (message.type == MessageParser::Type::Shorten)
  {
    if (!circuitBreaker_.allowRequest())
    {
      std::cout << "[CB] bloqueando ENCURTA: circuito "
                << CircuitBreaker::stateName(circuitBreaker_.state())
                << "\n";
      sendLine("ERR 503 Circuit open\n");
      return;
    }

    std::cout << "[PROXY] encaminhando ENCURTA para REST\n";
    auto result = restClient_.shorten(message.argument);
    if (!result.ok)
    {
      circuitBreaker_.recordFailure();
      std::cout << "[CB] falha ao chamar REST. estado="
                << CircuitBreaker::stateName(circuitBreaker_.state())
                << "\n";
      sendLine("ERR 502 Bad gateway\n");
      return;
    }
    circuitBreaker_.recordSuccess();
    std::cout << "[CB] sucesso ao chamar REST. estado="
              << CircuitBreaker::stateName(circuitBreaker_.state())
              << "\n";
    sendLine("OK " + result.code + " " + result.shortUrl + "\n");
    return;
  }

  if (message.type == MessageParser::Type::Resolve)
  {
    std::string cached;
    if (cache_.get(message.argument, cached))
    {
      std::cout << "[CACHE] HIT codigo=" << message.argument
                << " tamanho=" << cache_.size()
                << "/" << cache_.capacity() << "\n";
      sendLine("OK " + cached + "\n");
      return;
    }

    std::cout << "[CACHE] MISS codigo=" << message.argument
              << " tamanho=" << cache_.size()
              << "/" << cache_.capacity()
              << ". Consultando REST\n";

    if (!circuitBreaker_.allowRequest())
    {
      std::cout << "[CB] bloqueando RESOLVE: circuito "
                << CircuitBreaker::stateName(circuitBreaker_.state())
                << "\n";
      sendLine("ERR 503 Circuit open\n");
      return;
    }

    auto result = restClient_.resolve(message.argument);
    if (!result.ok)
    {
      if (result.error == "Not found")
      {
        std::cout << "[PROXY] REST retornou 404 para codigo="
                  << message.argument << "\n";
        sendLine("ERR 404 Not found\n");
      }
      else
      {
        circuitBreaker_.recordFailure();
        std::cout << "[CB] falha ao chamar REST. estado="
                  << CircuitBreaker::stateName(circuitBreaker_.state())
                  << "\n";
        sendLine("ERR 502 Bad gateway\n");
      }
      return;
    }

    circuitBreaker_.recordSuccess();
    if (!cache_.contains(message.argument) && cache_.size() >= cache_.capacity())
    {
      std::cout << "[CACHE] FULL capacidade=" << cache_.capacity()
                << ". A entrada menos recente sera removida\n";
    }
    cache_.put(message.argument, result.originalUrl);
    std::cout << "[CACHE] STORE codigo=" << message.argument
              << " tamanho=" << cache_.size()
              << "/" << cache_.capacity() << "\n";
    std::cout << "[CB] sucesso ao chamar REST. estado="
              << CircuitBreaker::stateName(circuitBreaker_.state())
              << "\n";
    sendLine("OK " + result.originalUrl + "\n");
    return;
  }

  if (message.type == MessageParser::Type::Remove)
  {
    if (!circuitBreaker_.allowRequest())
    {
      std::cout << "[CB] bloqueando REMOVE: circuito "
                << CircuitBreaker::stateName(circuitBreaker_.state())
                << "\n";
      sendLine("ERR 503 Circuit open\n");
      return;
    }

    std::cout << "[PROXY] encaminhando REMOVE para REST\n";
    auto result = restClient_.remove(message.argument);
    if (!result.ok)
    {
      if (result.error == "Not found")
      {
        std::cout << "[PROXY] REST retornou 404 para codigo="
                  << message.argument << "\n";
        sendLine("ERR 404 Not found\n");
      }
      else
      {
        circuitBreaker_.recordFailure();
        std::cout << "[CB] falha ao chamar REST. estado="
                  << CircuitBreaker::stateName(circuitBreaker_.state())
                  << "\n";
        sendLine("ERR 502 Bad gateway\n");
      }
      return;
    }

    circuitBreaker_.recordSuccess();
    cache_.erase(message.argument);
    std::cout << "[CACHE] INVALIDATE codigo=" << message.argument
              << " tamanho=" << cache_.size()
              << "/" << cache_.capacity() << "\n";
    std::cout << "[CB] sucesso ao chamar REST. estado="
              << CircuitBreaker::stateName(circuitBreaker_.state())
              << "\n";
    sendLine("OK\n");
    return;
  }

  sendLine("ERR 400 Bad request\n");
}
