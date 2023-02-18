#include "processor.h"

pParserError ExpressionStatementProc::verify(pToken t) {
  if (!check(t)) {
    return newPError("ExpressionStatement: Pre-processed token is wrong type: " + t->typeToString());
  }
  auto est = std::dynamic_pointer_cast<PreExpressionStatement>(t);

  auto exprProcessor = ExpressionProc(scope);
  return exprProcessor.verifyTokens(est->tokens);
}

pAstNode ExpressionStatementProc::create(pToken t) {
  auto est = std::dynamic_pointer_cast<PreExpressionStatement>(t);
  auto exprToken = est->tokens.front();
  auto exprProcessor = ExpressionProc(scope);
  auto stmt = std::make_shared<AstExpressionStatement>(t, std::dynamic_pointer_cast<AstExpression>(exprProcessor.create(exprToken)));
  return stmt;
}


pParserError ExpressionProc::verify(pToken t) {
  if (!check(t)) {
    return newPError("Expression Processor: Wrong pre-processor token " + t->typeToString());
  }

  auto expr = std::dynamic_pointer_cast<PreExpression>(t);
  pToken lastToken = nullptr;

  for (auto it = expr->tokens.begin(); it != expr->tokens.end(); ++it) {
    pToken tt = *it;
    if (lastToken == nullptr) {
      if (tt->isTypeOf(prefixTypes)) {
        lastToken = tt;
        continue;
      }

      if (tt->isTypeOf(callTypes)) {

        auto callProc = CallProc(scope);
        auto foundCall = callProc.checkLine(expr->tokens, it);

        if (foundCall) {
          lastToken = tt;
          continue;
        } else {
          return newPError("Expression Processor: unknown identifier: " + tt->verboseToken());
        }
      }

      return newPError("Expression Processor: Expected prefix token or identifier but got "
        + tt->typeToString() +
        ", literal: " + tt->literal);

    } else {
      if (lastToken->isTypeOf(infixTypes) && tt->isTypeOf(infixTypes) && !tt->isTypeOf(TokenType::MINUS)) {
        return newPError("Expression Processor: unexpected two infix types in a row "
          + lastToken->typeToString() + " and " + tt->typeToString());
      }

      if (lastToken->isTypeOf(TokenType::NUMBER) && tt->isTypeOf(TokenType::NUMBER)) {
        return newPError("Expression Processor: unexpected two numbers in a row "
          + lastToken->literal + " and " + tt->literal);
      }

      if (tt->isTypeOf(callTypes)) {
        if (lastToken->isTypeOf(TokenType::IDENTIFIER)) {
          return newPError("Expression Processor: unexpected two identifiers in a row "
            + lastToken->verboseToken() + " and " + tt->verboseToken());
        }

        auto callProc = CallProc(scope);
        auto callFound = callProc.checkLine(expr->tokens, it);

        if (callFound > 0) {
          lastToken = tt;
          it = std::next(it);
          continue;
        } else {
          return newPError("Expression Processor: unknown identifier: " + tt->verboseToken());
        }
      }
    }

    lastToken = tt;
  }

  return nullptr;
}

pAstNode ExpressionProc::create(pToken t) {
  auto exprToken = std::dynamic_pointer_cast<PreExpression>(t);
  src = exprToken->tokens;
  cur = src.begin();

  return parseExpression(ExprOrder::LOWEST);
}

std::shared_ptr<AstExpression> ExpressionProc::parseExpression(ExprOrder precedence) {
  std::shared_ptr<AstExpression> leftExp;

  auto callProc = CallProc(scope);

  if (callProc.checkLine(src, cur)) {
    leftExp = callProc.create(src, cur);
  } else {
    switch ((*cur)->type) {
      case TokenType::NUMBER:
      {
        leftExp = parseNumberLiteral();
        break;
      }
      case TokenType::BANG:
      case TokenType::MINUS:
      {
        leftExp = parsePrefixExpression();
        break;
      }
      default:
        // this is an critical error
        std::cerr << "Expression: Critical error: No prefix processor found for type: "
          << tokenTypeToString[(*cur)->type] << "\n";
        std::exit(103);
    }
  }

  for (;cur != src.end() && (cur + 1) != src.end() && precedence < peekPrecedence();) {
    auto peek = *(cur + 1);
    if (peek->isTypeOf(infixTypes)) {
      next();
      leftExp = parseInfixExpression(leftExp);
      continue;
    } else {
      std::cerr << "did not recognize infix type: " << peek->typeToString() << "\n";
    }
    return leftExp;
  }

  return leftExp;
}

std::shared_ptr<AstExpression> ExpressionProc::parseNumberLiteral() {
  return std::make_shared<AstNumber>((*cur), std::stod((*cur)->literal));
}

std::shared_ptr<AstExpression> ExpressionProc::parsePrefixExpression() {
  auto prefixExpression = std::make_shared<AstPrefixExpression>();
  prefixExpression->token = *cur;
  next();
  prefixExpression->right = parseExpression(ExprOrder::PREFIX);
  return prefixExpression;
}

std::shared_ptr<AstExpression> ExpressionProc::parseInfixExpression(std::shared_ptr<AstExpression> leftExp) {
  auto infixExpression = std::make_shared<AstInfixExpression>();
  infixExpression->left = leftExp;
  infixExpression->token = *cur;
  auto precedence = curPrecedence();
  next();
  infixExpression->right = parseExpression(precedence);
  return infixExpression;
}

pParserError DeclarationProc::verify(pToken t) {
  auto decl = std::dynamic_pointer_cast<PreDeclare>(t);
  auto tokens = decl->tokens;
  if (tokens.size() != 1) {
    return newPError("Declaration: Identifier is missing: " + t->verboseToken());
  }

  auto idProc = IdentifierProc(scope);
  return idProc.verify(tokens[0]);
}

pAstNode DeclarationProc::create(pToken t) {
  auto idProc = IdentifierProc(scope);
  auto decl = std::dynamic_pointer_cast<PreDeclare>(t);
  auto tokens = decl->tokens;
  // unfortunately declaration block needs to be handled in upper level (effectively in Parser loop).
  return std::make_shared<AstDeclarationStatement>(t, std::dynamic_pointer_cast<AstIdentifier>(idProc.create(tokens[0])));
}

pParserError IdentifierProc::verify(pToken t) {
  if (!check(t)) {
    return newPError("Identifier: expected identifier token, got " + t->typeToString());
  }
  auto idToken = std::dynamic_pointer_cast<PreIdentifier>(t);

  for (auto tt : idToken->tokens) {
    if (tt->isTypeOf(TokenType::WORD)) {
      continue;
    }

    if (tt->isTypeOf(TokenType::PARAMETER)) {
      continue;
    }

    return newPError("Identifier: Illigal token type: " + tt->typeToString());
  }

  return nullptr;
}
pAstNode IdentifierProc::create(pToken t) {
  auto idToken = std::dynamic_pointer_cast<PreIdentifier>(t);
  return std::make_shared<AstIdentifier>(t, idToken);
}

pParserError ExpressionProc::verifyTokens(Tokens tokens) {
  for (auto t : tokens) {
    auto error = verify(t);

    if (error != nullptr) {
      return error;
    }
  }

  return nullptr;
}

std::shared_ptr<PreIdentifier> CallProc::checkLine(Tokens tokens, Tokens::iterator it) {
  return checkIdentifiersForLinePerScope(tokens, it, scope);
}

std::shared_ptr<PreIdentifier> CallProc::checkIdentifiersForLinePerScope(Tokens tokens, Tokens::iterator it, pPreScope s) {
  for (auto id : s->identifiers) {
    auto found = checkIdentifier(tokens, it, id);
    if (found) {
      return found;
    }
  }

  if (s->broader != nullptr) {
    return checkIdentifiersForLinePerScope(tokens, it, s->broader);
  }

  return nullptr;
}

std::shared_ptr<PreIdentifier> CallProc::checkIdentifier(Tokens tokens, Tokens::iterator it, pPreIdentifier id) {
  auto identifier = std::dynamic_pointer_cast<PreIdentifier>(*it);

  if (id->tokens.size() != identifier->tokens.size()) {
    return nullptr;
  }
  for (u_int i = 0; i < identifier->tokens.size(); ++i) {
    auto idToken = id->tokens[i];
    auto lineToken = identifier->tokens[i];

    if (idToken->isTypeOf(TokenType::PARAMETER) && lineToken->isTypeOf(TokenType::PARAMETER)) {
      continue;
    }

    if (idToken != lineToken) {
      return nullptr;
    }
  }

  return id;
}

pAstCall CallProc::create(Tokens tokens, Tokens::iterator& it) {
  auto identifier = std::dynamic_pointer_cast<PreIdentifier>(*it);
  auto call = std::make_shared<AstCall>((*it), identifier);
  it = std::next(it);
  return call;
}