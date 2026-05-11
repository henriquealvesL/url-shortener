#include "MessageParser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
  std::string trim(const std::string &input)
  {
    auto start = std::find_if_not(input.begin(), input.end(), [](unsigned char ch)
                                  { return std::isspace(ch); });
    auto end = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char ch)
                                { return std::isspace(ch); })
                   .base();

    if (start >= end)
    {
      return {};
    }
    return std::string(start, end);
  }
}

MessageParser::ParseResult MessageParser::parseLine(const std::string &line)
{
  ParseResult result;
  std::string trimmed = trim(line);
  if (trimmed.empty())
  {
    result.error = "Empty command";
    return result;
  }

  std::istringstream iss(trimmed);
  std::string verb;
  if (!(iss >> verb))
  {
    result.error = "Malformed command";
    return result;
  }

  std::string arg;
  std::getline(iss, arg);
  arg = trim(arg);

  if (verb == "ENCURTA")
  {
    if (arg.empty())
    {
      result.error = "Missing URL";
      return result;
    }
    result.ok = true;
    result.message.type = Type::Shorten;
    result.message.argument = arg;
    return result;
  }

  if (verb == "RESOLVE")
  {
    if (arg.empty())
    {
      result.error = "Missing code";
      return result;
    }
    result.ok = true;
    result.message.type = Type::Resolve;
    result.message.argument = arg;
    return result;
  }

  if (verb == "REMOVE")
  {
    if (arg.empty())
    {
      result.error = "Missing code";
      return result;
    }
    result.ok = true;
    result.message.type = Type::Remove;
    result.message.argument = arg;
    return result;
  }

  result.error = "Unknown command";
  return result;
}
