#pragma once
#include <iostream>
#include "token.h"

class Lexer
{
public:
  Lexer() {}
  Lexer(std::string src) : source(src) {}
  void generateTokens();
  void addToken();
  void addWordToken();
  void addNumberToken();

  std::string source = "";
  Tokens tokens;
  u_int pos = 0;
};

bool isWordLetter(char c);
bool isLetter(char c);
bool isNumber(char c);