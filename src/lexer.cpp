#include "lexer.h"
#include "errors.h"
#include <iostream>

void Lexer::generateTokens()
{
  tokens = {};
  while (pos < source.length())
  {
    addToken();
    ++pos;
  }
  tokens.push_back(std::make_shared<EndOfFile>());
  token::verboseTokens(tokens);
}

void Lexer::addToken()
{
  auto c = source[pos];
  char peekToken = 0;
  if ((pos + 1) < source.length())
  {
    peekToken = source[pos + 1];
  }

  switch (c)
  {
  case ' ':
    return;
  case '\n':
    tokens.push_back(std::make_shared<EndL>());
    return;
  case '#':
    tokens.push_back(std::make_shared<Hash>());
    return;
  case '-':
    tokens.push_back(std::make_shared<Minus>());
    return;
  case '+':
    tokens.push_back(std::make_shared<Plus>());
    return;
  case '*':
    tokens.push_back(std::make_shared<Asterisk>());
    return;
  case '/':
    tokens.push_back(std::make_shared<Slash>());
    return;
  case '=':
    if (peekToken == '=')
    {
      tokens.push_back(std::make_shared<EqualsCompare>());
      ++pos;
      return;
    }
    tokens.push_back(std::make_shared<Equals>());
    return;
  case '(':
    tokens.push_back(std::make_shared<LBrace>());
    return;
  case ')':
    tokens.push_back(std::make_shared<RBrace>());
    return;
  case '[':
    tokens.push_back(std::make_shared<LBracket>());
    return;
  case ']':
    tokens.push_back(std::make_shared<RBracket>());
    return;
  case ':':
    tokens.push_back(std::make_shared<Colon>());
    return;
  case '!':
    if (peekToken == '=')
    {
      tokens.push_back(std::make_shared<NotEquals>());
      ++pos;
      return;
    }
    tokens.push_back(std::make_shared<Bang>());
    return;
  case '>':
    if (peekToken == '=')
    {
      tokens.push_back(std::make_shared<GtOrEquals>());
      ++pos;
      return;
    }
    tokens.push_back(std::make_shared<Gt>());
    return;
  case '<':
    if (peekToken == '=')
    {
      tokens.push_back(std::make_shared<LtOrEquals>());
      ++pos;
      return;
    }
    tokens.push_back(std::make_shared<Lt>());
    return;
  default:
    if (isLetter(c))
      addWordToken();
    else if (isNumber(c))
      addNumberToken();
    else
      tokens.push_back(std::make_shared<Token>(std::string(1, c)));
    return;
  }
}

void Lexer::addWordToken()
{
  u_int startingPos = pos;
  while (pos < source.length() && isWordLetter(source[pos]))
  {
    ++pos;
  }

  std::string literal = source.substr(startingPos, pos - startingPos);
  tokens.push_back(std::make_shared<Word>(literal));
  --pos;
}

void Lexer::addNumberToken()
{
  u_int startingPos = pos;
  u_int dots = 0;
  while (pos < source.length() && isNumber(source[pos]))
  {
    if (source[pos] == '.')
    {
      ++dots;
      if (dots > 1)
      {
        throw SYNTAX_ERROR("Number has more than one dot");
      }
    }

    ++pos;
  }

  std::string literal = source.substr(startingPos, pos - startingPos);
  tokens.push_back(std::make_shared<Number>(literal));
  --pos;
}

bool isLetter(char c)
{
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         c == '_';
}

bool isWordLetter(char c)
{
  return isLetter(c) ||
         isNumber(c) ||
         c == '.' ||
         c == '-';
}

bool isNumber(char c)
{
  return (c >= '0' && c <= '9') || c == '.';
}