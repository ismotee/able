#include "scopeBuilder.h"

void ScopeBuilder::build(Tokens &_tokens)
{
  for (auto t : _tokens)
  {
    if (t->isTypeOf(TokenType::BLOCK))
    {
      // safe since type is checked with type property
      auto block = std::dynamic_pointer_cast<PreBlock>(t);

      if (current == nullptr)
      {
        current = std::make_shared<PreScope>();
        current->depth = block->depth;

        if (root == nullptr)
        {
          root = current;
        }
      }
      else
      {
        while (block->depth < current->depth)
        {
          current = current->broader;
        }

        auto scope = std::make_shared<PreScope>(current);
        current->narrower.push_back(scope);
        scope->depth = block->depth;
        current->tokens.push_back(scope);
        current = scope;
      }
    }
    else
    {
      current->tokens.push_back(t);
    }
  }
}