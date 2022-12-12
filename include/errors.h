#pragma once
#include <exception>
#include <string>

class SYNTAX_ERROR: public std::exception {
public:
  SYNTAX_ERROR(std::string description): desc(description) {}

  const char* what() const throw() {
    return desc.c_str();
  }

  std::string desc = "unknown error";
};

class PREFIX_MISSING: public std::exception {
public:
  PREFIX_MISSING() {}
  PREFIX_MISSING(std::string description): desc(description) {}

  const char* what() const throw() {
    return desc.c_str();
  }

  std::string desc = "Expression is missing a prefix expression.";
};

class INTERNAL_ERROR: public std::exception {
public:
  INTERNAL_ERROR() {}
  INTERNAL_ERROR(std::string description): desc(description) {}

  const char* what() const throw() {
    return (body + desc).c_str();
  }
  std::string body = "Internal error:";
  std::string desc;
};