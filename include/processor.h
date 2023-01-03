#pragma once
#include <string>
#include "token.h"
#include "ast.h"

enum class ExprOrder
{
  LOWEST = 0,
  EQUALS,
  LESSGREATER,
  SUM,
  PPRODUCT,
  PREFIX
};

struct ParserError
{
  ParserError(std::string _msg) : msg(_msg) {}

  std::string msg;
};

typedef std::shared_ptr<ParserError> pParserError;

class Processor
{
public:
  bool check(pToken t) { return type == t->type; }
  virtual pParserError verify(pToken t) = 0;
  virtual pAstNode create(pToken t) = 0;

protected:
  pParserError newPError(std::string msg)
  {
    return std::make_shared<ParserError>(msg);
  }

  TokenType type;
};

typedef std::shared_ptr<Processor> pProcessor;
typedef std::vector<pProcessor> Processors;

class ExpressionStatementProcessor : public Processor
{
public:
  pParserError verify(pToken t);
  pAstNode create(pToken t);

  TokenType type = TokenType::EXPRESSION_STATEMENT;
};

class ExpressionProcessor : public Processor
{
public:
  ExpressionProcessor(ExprOrder _order = ExprOrder::LOWEST) : order(_order) {}
  pParserError verify(pToken t);
  pAstNode create(pToken t);
  std::shared_ptr<AstExpression> parseExpression();

  TokenType type = TokenType::EXPRESSION;
  ExprOrder order;
  Tokens src;
  Tokens::iterator cur;

  std::vector<TokenType> prefixTypes = {TokenType::IMPORT, TokenType::BANG, TokenType::NUMBER, TokenType::MINUS};
  std::vector<TokenType> infixTypes = {TokenType::PLUS, TokenType::MINUS, TokenType::ASTERISK, TokenType::SLASH};
};