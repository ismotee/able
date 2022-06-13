#pragma once
#include <exception>
#include <string>

class SYNTAX_ERROR : public std::exception
{
public:
  SYNTAX_ERROR(std::string description) : desc(description) {}

  const char *what() const throw()
  {
    return desc.c_str();
  }

  std::string desc = "unknown error";
};