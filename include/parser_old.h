#pragma once
#include <memory>
#include <iterator>
#include "ast.h"
#include "token.h"
#include "tokenType.h"

enum class ExprOrder
{
  LOWEST = 0,
  EQUALS,
  LESSGREATER,
  SUM,
  PPRODUCT,
  PREFIX
};

static std::unordered_map<TokenType, ExprOrder> precedences = {
    {TokenType::EQUALS_COMPARE, ExprOrder::EQUALS},
    {TokenType::NOT_EQUALS, ExprOrder::EQUALS},
    {TokenType::LT, ExprOrder::LESSGREATER},
    {TokenType::GT, ExprOrder::LESSGREATER},
    {TokenType::LT_OR_EQUALS, ExprOrder::LESSGREATER},
    {TokenType::GT_OR_EQUALS, ExprOrder::LESSGREATER},
    {TokenType::PLUS, ExprOrder::SUM},
    {TokenType::MINUS, ExprOrder::SUM},
    {TokenType::SLASH, ExprOrder::PPRODUCT},
    {TokenType::ASTERISK, ExprOrder::PPRODUCT},
};

static std::string trimTrailingWhiteSpace(std::string str)
{
  while (str.size() > 0 && str[str.size() - 1] == ' ')
  {
    str.pop_back();
  }
  return str;
}

class Parser
{
public:
  Parser(pPreScope root = nullptr) : preScope(root), scope(std::make_shared<AstNode>()) {}

  void parse();
  pPreScope preScope;
  pAstNode scope;
  Tokens::iterator cur;

protected:
  void parseExpressionStatement();
  std::shared_ptr<AstExpression> parseExpression(ExprOrder precedence);
  std::shared_ptr<AstExpression> parsePrefixExpression();
  std::shared_ptr<AstExpression> parseInfixExpression(std::shared_ptr<AstExpression> leftExp);
  std::shared_ptr<AstExpression> parseNumberLiteral();
  /*
      void parseStatement();
      void parseCall();
      void parseKeyPhraseCall() {}
      void buildBlock();
      void buildDefinition();
      pPhrase buildPhrase();
      bool isPhrase();
      bool isKeyPhrase() { return false; }
      ExprOrder curPrecedence();
      ExprOrder peekPrecedence();
  */

private:
  void next()
  {
    cur = std::next(cur);
  }
};
