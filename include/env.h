#pragma once
#include "ast.h"

struct Env
{

  std::vector<std::shared_ptr<AstIdentifier>> identifiers;

  std::shared_ptr<AstIdentifier> findIdentifier(Tokens::iterator it)
  {
    for (auto identifier : identifiers)
    {
      if (identifier->compare(it))
      {
        return identifier;
      }
    }
    return nullptr;
  }
};