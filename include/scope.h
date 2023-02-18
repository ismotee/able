#pragma once
#include <memory>
#include <vector>
#include "token.h"

class Scope
{
public:
  Scope() {}
};

typedef std::shared_ptr<Scope> ptrScope;
