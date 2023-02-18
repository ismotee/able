#pragma once
#include <string>
#include <unordered_map>
#include "token.h"
#include "ast.h"

enum class ExprOrder {
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

struct ParserError {
  ParserError(std::string _msg): msg(_msg) {}

  std::string msg;
};

typedef std::shared_ptr<ParserError> pParserError;

class Processor {
public:
  Processor(TokenTypes _type, pPreScope _scope): type(_type), scope(_scope) {}
  virtual bool check(pToken t) {
    return t->isTypeOf(type);
  }
  virtual pParserError verify(pToken t) = 0;
  virtual pAstNode create(pToken t) = 0;

protected:
  pParserError newPError(std::string msg) {
    return std::make_shared<ParserError>(msg);
  }

  TokenTypes type;
  pPreScope scope;
};

typedef std::shared_ptr<Processor> pProcessor;
typedef std::vector<pProcessor> Processors;

class ExpressionStatementProc: public Processor {
public:
  ExpressionStatementProc(pPreScope scope): Processor({ TokenType::EXPRESSION_STATEMENT }, scope) {}
  pParserError verify(pToken t);
  pAstNode create(pToken t);

};

class DeclarationProc: public Processor {
public:
  DeclarationProc(pPreScope scope): Processor({ TokenType::DECLARE }, scope) {}
  pParserError verify(pToken t);
  pAstNode create(pToken t);
};

class ExpressionProc: public Processor {
public:
  ExpressionProc(pPreScope scope): Processor({ TokenType::EXPRESSION }, scope) {}
  pParserError verify(pToken t);
  pAstNode create(pToken t);

  pParserError verifyTokens(Tokens t);
  std::shared_ptr<AstExpression> parseExpression(ExprOrder precedence);
  std::shared_ptr<AstExpression> parseNumberLiteral();
  std::shared_ptr<AstExpression> parsePrefixExpression();
  std::shared_ptr<AstExpression> parseInfixExpression(std::shared_ptr<AstExpression> leftExp);

  Tokens src;
  Tokens::iterator cur;

  TokenTypes prefixTypes = { TokenType::IMPORT, TokenType::BANG, TokenType::NUMBER, TokenType::MINUS, TokenType::CALL };
  TokenTypes infixTypes = { TokenType::PLUS, TokenType::MINUS,
    TokenType::ASTERISK, TokenType::SLASH,
    TokenType::EQUALS_COMPARE, TokenType::NOT_EQUALS,
    TokenType::GT, TokenType::GT_OR_EQUALS,
    TokenType::LT, TokenType::LT_OR_EQUALS };
  TokenTypes callTypes = { TokenType::IDENTIFIER };

  void next() {
    if (cur != src.end()) {
      cur = std::next(cur);
    }
    return;
  }

  ExprOrder peekPrecedence() {
    if (cur == src.end() || (cur + 1) == src.end()) {
      return ExprOrder::LOWEST;
    }

    try {
      return precedences.at((*(cur + 1))->type);
    } catch (std::out_of_range& e) {
      std::cerr << "Expression: Could not find precedence type for " << (*cur)->literal
        << "(" << (*cur)->typeToString() << ")\n";
      return ExprOrder::LOWEST;
    }

  }

  ExprOrder curPrecedence() {
    try {
      return precedences.at((*cur)->type);
    } catch (std::out_of_range& e) {
      return ExprOrder::LOWEST;
    }
  }
};

class IdentifierProc: public Processor {
public:
  IdentifierProc(pPreScope scope): Processor({ TokenType::IDENTIFIER }, scope) {}
  pParserError verify(pToken t);
  pAstNode create(pToken t);
};

class CallProc {
public:
  CallProc(pPreScope _scope): scope(_scope) {}
  // important! Use checkLine instead of basic check. 
  // Identifier check needs the whole line rather than single token.
  // checkLine returns 0 if identifier not found 
  std::shared_ptr<PreIdentifier> checkLine(Tokens tokens, Tokens::iterator it);
  std::shared_ptr<AstCall> create(Tokens tokens, Tokens::iterator& it);

  std::shared_ptr<PreIdentifier> checkIdentifiersForLinePerScope(Tokens tokens, Tokens::iterator it, pPreScope s);
  std::shared_ptr<PreIdentifier> checkIdentifier(Tokens tokens, Tokens::iterator it, pPreIdentifier t);

  pPreScope scope;
};

