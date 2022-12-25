#pragma once
#include <memory>
#include <vector>
#include "tokenType.h"

struct Token;
typedef std::vector<std::shared_ptr<Token>> Tokens;
typedef std::shared_ptr<Token> pToken;

struct Token
{
  Token(TokenType t = TokenType::UNDEFINED) : type(t), literal(tokenTypeToString[t]) {}
  Token(
      std::string l,
      TokenType t = TokenType::UNDEFINED) : type(t), literal(l) {}

  virtual std::string verboseToken()
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

  bool isTypeOf(TokenType comp)
  {
    return type == comp;
  }

  bool operator==(const Token &t)
  {
    return type == t.type && literal == t.literal;
  }

  bool operator!=(const Token &t)
  {
    return !(type == t.type && literal == t.literal);
  }

  TokenType type;
  std::string literal;
};

inline bool operator==(const pToken &lht, const pToken &rht)
{
  return *lht == *rht;
}

inline bool operator!=(const pToken &lht, const pToken &rht)
{
  return *lht != *rht;
}

// some helpers

namespace token
{
  const std::vector<TokenType> END = {TokenType::ENDL, TokenType::END_OF_FILE};
  inline void linkTokens(Tokens &tokens)
  {
    /*
    if (tokens.size() < 2) {
      return;
    }

    for (u_int i = 1; i < tokens.size(); ++i) {
      tokens[i - 1]->next = tokens[i];
      tokens[i]->prev = tokens[i - 1];
    }
    */
  }

  inline std::string verboseTokens(Tokens &tokens)
  {
    std::string verbal = "";
    for (auto token : tokens)
    {
      if (token->type == TokenType::ENDL && verbal.size() > 0)
      {
        verbal.pop_back();
      }

      verbal += token->verboseToken();
      if (token->type != TokenType::END_OF_FILE)
      {
        verbal += " ";
      }
    }
    verbal.pop_back();
    return verbal;
  }

};

class Hash : public Token
{
public:
  Hash() : Token(TokenType::HASH) {}
};

class Minus : public Token
{
public:
  Minus() : Token(TokenType::MINUS) {}
};

class Plus : public Token
{
public:
  Plus() : Token(TokenType::PLUS) {}
};

class Asterisk : public Token
{
public:
  Asterisk() : Token(TokenType::ASTERISK) {}
};

class Slash : public Token
{
public:
  Slash() : Token(TokenType::SLASH) {}
};

class Equals : public Token
{
public:
  Equals() : Token(TokenType::EQUALS) {}
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
  LBrace() : Token(TokenType::LBRACE) {}
};

class RBrace : public Token
{
public:
  RBrace() : Token(TokenType::RBRACE) {}
};

class LBracket : public Token
{
public:
  LBracket() : Token(TokenType::LBRACKET) {}
};

class RBracket : public Token
{
public:
  RBracket() : Token(TokenType::RBRACKET) {}
};

class Colon : public Token
{
public:
  Colon() : Token(TokenType::COLON) {}
};

class Bang : public Token
{
public:
  Bang() : Token(TokenType::BANG) {}
};

class EqualsCompare : public Token
{
public:
  EqualsCompare() : Token(TokenType::EQUALS_COMPARE) {}
};

class NotEquals : public Token
{
public:
  NotEquals() : Token(TokenType::NOT_EQUALS) {}
};

class Gt : public Token
{
public:
  Gt() : Token(TokenType::GT) {}
};

class Lt : public Token
{
public:
  Lt() : Token(TokenType::LT) {}
};

struct GtOrEquals : public Token
{
  GtOrEquals() : Token(TokenType::GT_OR_EQUALS) {}
};

struct LtOrEquals : public Token
{
  LtOrEquals() : Token(TokenType::LT_OR_EQUALS) {}
};

struct EndL : public Token
{
  EndL() : Token(TokenType::ENDL) {}
};

struct EndOfFile : public Token
{
  EndOfFile() : Token("", TokenType::END_OF_FILE) {}
};

// pre-semantic tokens

struct PreToken;
typedef std::shared_ptr<PreToken> pPreToken;
typedef std::vector<pPreToken> PreTokens;

struct PreToken : public Token
{
  PreToken(TokenType t = TokenType::PRE_TOKEN) : Token(t) {}

  std::string verbose()
  {
    std::string result;
    for (auto token : tokens)
    {
      result += token->verboseToken() + " ";
    }
    result.pop_back();
    return result;
  }

  virtual std::string verboseToken() override
  {
    return verbose();
  }

  Tokens tokens = {};
};

struct PreIdentifier : public PreToken
{
  PreIdentifier() : PreToken(TokenType::IDENTIFIER) {}
};

struct PreParameter : public PreToken
{
  PreParameter() : PreToken(TokenType::PARAMETER) {}
};

struct PreDeclare : public PreToken
{
  PreDeclare() : PreToken(TokenType::DECLARE) {}

  std::string verboseToken()
  {
    return "# " + token::verboseTokens(tokens);
  }
};

struct PreBlock : public PreToken
{
  PreBlock() : PreToken(TokenType::BLOCK), depth(0) {}

  std::string verboseToken() override
  {
    std::string result = "(" + std::to_string(depth) + ") {}\n";
    return result;
  }

  u_int depth;
};

// deprecated

class Import : public Token
{
public:
  Import() : Token(TokenType::IMPORT) {}
};

class From : public Token
{
public:
  From() : Token(TokenType::FROM) {}
};

class Assign : public Token
{
public:
  Assign() : Token(TokenType::ASSIGN) {}
};

class With : public Token
{
public:
  With() : Token(TokenType::WITH) {}
};

class Expression : public Token
{
public:
  Expression() : Token(TokenType::EXPRESSION) {}
};

class PrefixExpression : public Token
{
public:
  PrefixExpression() : Token(TokenType::EXPRESSION) {}

  Token oper;
  Expression right;
};

class Call : public Token
{
public:
  Call() : Token(TokenType::CALL) {}
};

class Argument : public Token
{
public:
  Argument() : Token(TokenType::ARGUMENT) {}
};

// Keyphrases

class Print : public Token
{
public:
  Print() : Token(TokenType::PRINT) {}
};

class Join : public Token
{
public:
  Join() : Token(TokenType::JOIN) {}
};
