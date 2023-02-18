#pragma once
#include "token.h"

class ScopeBuilder
{
public:
  ScopeBuilder() {}
  void build(Tokens &_tokens);

  pPreScope root;
  pPreScope current;
};