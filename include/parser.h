#pragma once
#include <memory>
#include <iterator>
#include "ast.h" 
#include "token.h"
#include "tokenType.h" 

class Parser {
public:
  Parser(pPreScope p = nullptr): pre(p) {}
  void parseScope();
  void parseStatement(pToken p);
  pParserError verifyStatement(pToken t);
  pAstStatement createStatement(pToken t);

  pPreScope pre;
  pAstProgram scope = std::make_shared<AstProgram>();
  Processors processors = {
      std::make_shared<ExpressionStatementProc>(pre),
      std::make_shared<DeclarationProc>(pre) };

protected:
  pParserError newPError(std::string msg) {
    return std::make_shared<ParserError>(msg);
  }

  Tokens::iterator cur;
  Tokens::iterator prev;
};
