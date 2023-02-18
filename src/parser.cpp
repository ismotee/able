#include "parser.h"

void Parser::parseScope() {
  pToken lastToken = nullptr;
  for (auto t : pre->tokens) {
    if (lastToken != nullptr && lastToken->isTypeOf(TokenType::DECLARE) && t->isTypeOf(TokenType::SCOPE)) {
      auto decl = std::dynamic_pointer_cast<AstDeclarationStatement>(scope->statements.back());
      auto scp = std::dynamic_pointer_cast<PreScope>(t);
      Parser p(scp);
      p.parseScope();
      decl->scope = p.scope;
    } else {
      parseStatement(t);
    }
    lastToken = t;
  }
}

void Parser::parseStatement(pToken t) {
  scope->addStatement(createStatement(t));
}

pAstStatement Parser::createStatement(pToken t) {
  for (auto processor : processors) {
    if (processor->check(t)) {
      auto error = processor->verify(t);

      if (error == nullptr) {
        auto stmt = std::dynamic_pointer_cast<AstStatement>(processor->create(t));
        return stmt;
      } else {
        std::cerr << "Parser: tried to parse token of type " << t->literal << " but parser run to an error:\n";
        std::cerr << "  " << error->msg << "\n";
        std::exit(102);
      }
      phrase->params.push_back(param);
      continue;
    }

    phrase->tokens.push_back(*cur);
  }

  return phrase;
}

ExprOrder Parser::curPrecedence() {
  try {
    return precedences.at((*cur)->type);
  } catch (std::out_of_range e) {
    return ExprOrder::LOWEST;
  }
}

ExprOrder Parser::peekPrecedence() {
  if (cur == tokens.end() || (cur + 1) == tokens.end()) {
    return ExprOrder::LOWEST;
  }

  try {
    return precedences.at((*(cur + 1))->type);
  } catch (std::out_of_range e) {
    std::cout << "no supposed to be here\n";
    return ExprOrder::LOWEST;
  }
}