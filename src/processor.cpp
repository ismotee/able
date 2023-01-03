#include "processor.h"

pParserError ExpressionStatementProcessor::verify(pToken t)
{
  if (!check(t))
  {
    return newPError("ExpressionStatement: Pre-processed token is wrong type: " + tokenTypeToString[t->type]);
  }

  auto est = std::dynamic_pointer_cast<PreExpressionStatement>(t);

  for (auto t : est->tokens)
  {
    auto exprProcessor = ExpressionProcessor();
    auto error = exprProcessor.verify(t);

    if (error != nullptr)
    {
      return error;
    }
  }

  return nullptr;
}

pAstNode ExpressionStatementProcessor::create(pToken t)
{
  auto exprProcessor = ExpressionProcessor();
  auto stmt = std::make_shared<AstExpression>(t, exprProcessor.create(t));
  return stmt;
}

pParserError ExpressionProcessor::verify(pToken t)
{
  if (!check(t))
  {
    return newPError("Expression Processor: Wronn pre-processor token " + tokenTypeToString[t->type]);
  }

  auto expr = std::dynamic_pointer_cast<PreExpression>(t);
  bool isLastInfix = false;

  for (auto tt : expr->tokens)
  {
    if (isLastInfix)
    {
      if (tt->isTypeOf(prefixTypes))
      {
        isLastInfix = false;
        continue;
      }
    }
    else
    {
      if (tt->isTypeOf(infixTypes))
      {
        isLastInfix = true;
        continue;
      }
      else if (!tt->isTypeOf({TokenType::MINUS, TokenType::BANG}))
      {
        isLastInfix = false;
        continue;
      }
      return newPError("Expression Processor: Unexpected token " + tokenTypeToString[tt->type]);
    }
  }

  if (isLastInfix)
  {
    return newPError("Expression Processor: unexpected end of expression " + tokenTypeToString[expr->tokens.back()->type]);
  }

  return nullptr;
}

pAstNode ExpressionProcessor::create(pToken t)
{
  if (!check(t))
  {
    return nullptr;
  }

  auto exprToken = std::dynamic_pointer_cast<PreExpression>(t);
  src = exprToken->tokens;
  cur = src.begin();

  return parseExpression();
}

std::shared_ptr<AstExpression> ExpressionProcessor::parseExpression()
{
  std::shared_ptr<AstExpression> leftExp;

  switch ((*cur)->type)
  {
  case TokenType::NUMBER:
  {
    leftExp = parseNumberLiteral();
    break;
  }
  }
}