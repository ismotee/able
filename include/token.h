#pragma once
#include <memory>
#include <vector>
#include "tokenType.h"

class Token;
typedef std::vector<std::shared_ptr<Token>> Tokens;

class Token
{
public:
  Token() {}
  Token(std::string l, TokenType t = TokenType::UNDEFINED) : literal(l), type(t) {}

  bool isPartOf(std::shared_ptr<Token> t)
  {
    return t == partOf;
  }

  std::string verboseToken()
  {
    return literal;
  }

  bool isTypeOf(std::vector<TokenType> types)
  {
    for (auto _type : types)
    {
      if (type == _type)
      {
        return true;
      }
    }
    return false;
  }

  std::string literal;
  TokenType type;
  std::shared_ptr<Token> partOf;
  std::shared_ptr<Token> next;
  std::shared_ptr<Token> relatedTo;
};

class Hash : public Token
{
public:
  Hash() : Token("#", TokenType::HASH) {}
};

class Minus : public Token
{
public:
  Minus() : Token("-", TokenType::MINUS) {}
};

class Plus : public Token
{
public:
  Plus() : Token("+", TokenType::PLUS) {}
};

class Asterisk : public Token
{
public:
  Asterisk() : Token("*", TokenType::ASTERISK) {}
};

class Slash : public Token
{
public:
  Slash() : Token("/", TokenType::SLASH) {}
};

class Equals : public Token
{
public:
  Equals() : Token("=", TokenType::EQUALS) {}
};

class Word : public Token
{
public:
  Word() {}
  Word(std::string l) : Token(l, TokenType::WORD) {}
};

class Number : public Token
{
public:
  Number() {}
  Number(std::string l) : Token(l, TokenType::NUMBER) {}
};

class LBrace : public Token
{
public:
  LBrace() : Token("(", TokenType::LBRACE) {}
};

class RBrace : public Token
{
public:
  RBrace() : Token(")", TokenType::RBRACE) {}
};

class LBracket : public Token
{
public:
  LBracket() : Token("[", TokenType::LBRACKET) {}
};

class RBracket : public Token
{
public:
  RBracket() : Token("]", TokenType::RBRACKET) {}
};

class Colon : public Token
{
public:
  Colon() : Token(":", TokenType::COLON) {}
};

class Bang : public Token
{
public:
  Bang() : Token("!", TokenType::BANG) {}
};

class EqualsCompare : public Token
{
public:
  EqualsCompare() : Token("==", TokenType::EQUALS_COMPARE) {}
};

class NotEquals : public Token
{
public:
  NotEquals() : Token("!=", TokenType::NOT_EQUALS) {}
};

class Gt : public Token
{
public:
  Gt() : Token(">", TokenType::GT) {}
};

class Lt : public Token
{
public:
  Lt() : Token("<", TokenType::LT) {}
};

class GtOrEquals : public Token
{
public:
  GtOrEquals() : Token(">=", TokenType::GT_OR_EQUALS) {}
};

class LtOrEquals : public Token
{
public:
  LtOrEquals() : Token("<=", TokenType::LT_OR_EQUALS) {}
};

class EndL : public Token
{
public:
  EndL() : Token("\n", TokenType::ENDL) {}
};

class EndOfFile : public Token
{
public:
  EndOfFile() : Token("", TokenType::END_OF_FILE) {}
};

// pre-semantic tokens

class Import : public Token
{
public:
  Import() : Token("IMPORT", TokenType::IMPORT) {}
};

class From : public Token
{
public:
  From() : Token("FROM", TokenType::FROM) {}
};

class Declare : public Token
{
public:
  Declare() : Token("DECLARE", TokenType::DECLARE) {}
};

class Assign : public Token
{
public:
  Assign() : Token("ASSIGN", TokenType::ASSIGN) {}
};

class With : public Token
{
public:
  With() : Token("WITH", TokenType::WITH) {}
};

class Identifier : public Token
{
public:
  Identifier() : Token("IDENTIFIER", TokenType::IDENTIFIER) {}
};

class Parameter : public Token
{
public:
  Parameter() : Token("PARAMETER", TokenType::PARAMETER) {}
};

class Expression : public Token
{
public:
  Expression() : Token("EXPRESSION", TokenType::EXPRESSION) {}
};

class Call : public Token
{
public:
  Call() : Token("CALL", TokenType::CALL) {}
};

class Argument : public Token
{
public:
  Argument() : Token("ARGUMENT", TokenType::ARGUMENT) {}
};
