#include "parser.h"
#include "errors.h"

void Parser::parse() {
  buildBlock();
}

void Parser::buildBlock() {
  for (; cur != tokens.end(); next()) {
    switch ((*cur)->type) {
      case TokenType::HASH:
      {
        u_int depth = 1;
        u_int pos = getPosition() - 1;

        for (next(); (*cur)->isTypeOf(TokenType::HASH); next()) depth++;

        if (depth > scope->getDepth()) {
          buildDefinition();
          continue;

        } else {
          setPosition(pos);
          return;
        }
      }
      case TokenType::END_OF_FILE:
      {
        setPosition(getPosition() - 1);
        return;
      }
      case TokenType::ENDL:
      {
        continue;
      }
      default:
        parseStatement();
        continue;
    }
  }
}

void Parser::parseStatement() {
  if (isPhrase()) {
    parseCall();
    return;
  }

  if (isKeyPhrase()) {
    parseKeyPhraseCall();
    return;
  }

  switch ((*cur)->type) {
    //    case TokenType::RETURN: {}
    default:
      parseExpressionStatement();
      return;
  }
}

void Parser::parseCall() {

}

void Parser::parseExpressionStatement() {
  auto stmt = std::make_shared<AstExpressionStatement>(*cur, parseExpression(ExprOrder::LOWEST));
  scope->addLine(stmt);
}

std::shared_ptr<AstExpression> Parser::parseExpression(ExprOrder precedence) {
  std::shared_ptr<AstExpression> leftExp;
  // prefix

  switch ((*cur)->type) {
    case TokenType::NUMBER:
    {
      leftExp = parseNumberLiteral();
      break;
    }
    case TokenType::MINUS:
    {
      leftExp = parsePrefixExpression();
      break;
    }
    case TokenType::BANG:
    {
      leftExp = parsePrefixExpression();
      break;
    }
    default:
      throw("expected prefix expression such as number, ! or -");
  }

  for (auto peek = *(cur + 1); !peek->isTypeOf({ TokenType::ENDL, TokenType::END_OF_FILE }) && precedence < peekPrecedence(); peek = *(cur + 1)) {
    switch (peek->type) {
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::ASTERISK:
      case TokenType::SLASH:
        next();
        leftExp = parseInfixExpression(leftExp);
        continue;
      default:
        return leftExp;
    }
    return leftExp;
  }
  return leftExp;
}

std::shared_ptr<AstExpression> Parser::parseNumberLiteral() {
  return std::make_shared<AstNumber>((*cur), std::stod((*cur)->literal));
}

std::shared_ptr<AstExpression> Parser::parseInfixExpression(std::shared_ptr<AstExpression> leftExp) {
  auto infixExpression = std::make_shared<AstInfixExpression>();
  infixExpression->left = leftExp;
  infixExpression->token = (*cur);
  auto precedence = curPrecedence();
  next();
  infixExpression->right = parseExpression(precedence);
  return infixExpression;
}

std::shared_ptr<AstExpression> Parser::parsePrefixExpression() {
  auto prefixExpression = std::make_shared<AstPrefixExpression>();
  prefixExpression->token = (*cur);
  next();
  prefixExpression->right = parseExpression(ExprOrder::PREFIX);
  return prefixExpression;
}

void Parser::buildDefinition() {
  auto phrase = buildPhrase();
  auto narrowerScope = Scope::New(scope, phrase);
  auto narrowerParser = Parser(tokens, narrowerScope);

  narrowerParser.setPosition(getPosition());
  narrowerParser.parse();
  scope->addLine(narrowerParser.scope);
  setPosition(narrowerParser.getPosition());
}

pPhrase Parser::buildPhrase() {
  pPhrase phrase = Phrase::New();
  for (; cur != tokens.end() && !(*cur)->isTypeOf({ TokenType::ENDL, TokenType::END_OF_FILE }); next()) {
    if ((*cur)->isTypeOf(TokenType::LBRACE)) {
      phrase->tokens.push_back(std::make_shared<Parameter>());
      pParam param = Param::New();
      for (next(); !(*cur)->isTypeOf(TokenType::RBRACE); next()) {
        param->tokens.push_back(*cur);
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
  } catch (std::out_of_range& e) {
    return ExprOrder::LOWEST;
  }
}

ExprOrder Parser::peekPrecedence() {
  if (cur == tokens.end() || (cur + 1) == tokens.end()) {
    return ExprOrder::LOWEST;
  }

  try {
    return precedences.at((*(cur + 1))->type);
  } catch (std::out_of_range& e) {
    throw INTERNAL_ERROR("Could not find precedence of the type");
    return ExprOrder::LOWEST;
  }
}