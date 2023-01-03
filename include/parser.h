#pragma once
#include "token.h"
#include "ast.h"
#include "processor.h"

class Parser
{
public:
  Parser() {}
  void parseScope(pPreScope s);
  void parseStatement(pToken p);
  pParserError verifyStatement(pToken t);
  pAstStatement createStatement(pToken t);

  pPreScope pre;
  pAstProgram scope;
  Processors processors = {
      std::make_shared<ExpressionStatementProcessor>()};

protected:
  pParserError newPError(std::string msg)
  {
    return std::make_shared<ParserError>(msg);
  }
};