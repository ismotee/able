#include "parser.h"

void Parser::parseScope(pPreScope s)
{
  for (auto t : pre->tokens)
  {
    parseStatement(t);
  }
}

void Parser::parseStatement(pToken t)
{
  auto error = verifyStatement(t);

  if (error == nullptr)
  {
    scope->addStatement(createStatement(t));
    return;
  }
}

pAstStatement Parser::createStatement(pToken t)
{
  for (auto processor : processors)
  {
    if (processor->check(t))
    {
      auto error = processor->verify(t);

      if (error == nullptr)
      {
        processor->create(t);
        return;
      }
    }
  }

  std::cerr << "Parser: cannot parse token of type " << t->literal << " (" << tokenTypeToString[t->type] << ")\n";
  std::exit(102);
}