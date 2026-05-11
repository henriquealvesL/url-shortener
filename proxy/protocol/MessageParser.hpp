#pragma once

#include <string>

class MessageParser
{
public:
  enum class Type
  {
    Shorten,
    Resolve,
    Remove,
    Invalid
  };

  struct Message
  {
    Type type{Type::Invalid};
    std::string argument;
  };

  struct ParseResult
  {
    bool ok{false};
    Message message{};
    std::string error;
  };

  static ParseResult parseLine(const std::string &line);
};
