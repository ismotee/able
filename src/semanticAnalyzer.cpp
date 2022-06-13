#include "semanticAnalyzer.h"
#include "errors.h"
#include <iostream>

void SemanticAnalyzer::analyze()
{
  handleDefinitions();
  prepareTokens();
  gatherDefinitions();
  handleCalls();
  prepareTokens();
  handleKeywords();
}

void SemanticAnalyzer::handleDefinitions()
{
  for (auto it = tokens.begin(); it != tokens.end(); ++it)
  {
    auto token = *it;
    switch (token->type)
    {
    case TokenType::LBRACKET:
      it = createLink(it);
      break;
    case TokenType::HASH:
      it = createDeclaration(it);
      break;
    case TokenType::EQUALS:
      it = createAssignment(it);
      break;
    default:
      analyzedTokens.push_back(token);
    }
  }
}

void SemanticAnalyzer::gatherDefinitions()
{
  for (auto it = tokens.begin(); it != tokens.end(); ++it)
  {
    auto token = *it;
    if (token->type == TokenType::IDENTIFIER)
    {
      definitions.push_back(token);
    }
  }
}

void SemanticAnalyzer::handleCalls()
{
  for (auto it = tokens.begin(); it != tokens.end(); ++it)
  {
    auto token = *it;
    switch (token->type)
    {
    case TokenType::WORD:
    {
      if (token->partOf == NULL)
      {
        it = searchIdentifier(it);
      }
      else
      {
        analyzedTokens.push_back(token);
      }
      break;
    }
    default:
      analyzedTokens.push_back(token);
    }
  }
}

void SemanticAnalyzer::handleKeywords()
{
  analyzedTokens = tokens;
}

void SemanticAnalyzer::prepareTokens()
{
  tokens = analyzedTokens;
  analyzedTokens = {};

  if (tokens.size() < 2)
  {
    return;
  }

  // link tokens
  for (u_int i = 1; i < tokens.size(); ++i)
  {
    std::cout << tokens[i - 1]->literal << "\n";
    tokens[i - 1]->next = tokens[i];
  }
}

template <typename Iterator>
Iterator SemanticAnalyzer::createLink(Iterator it)
{
  auto importToken = std::make_shared<Import>();
  analyzedTokens.push_back(importToken);
  it = createIdentifier(++it, importToken, {TokenType::RBRACKET});

  auto fromToken = std::make_shared<From>();
  analyzedTokens.push_back(fromToken);
  ++it;
  it = createIdentifier(++it, fromToken, {TokenType::RBRACKET});

  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::createDeclaration(Iterator it)
{
  if ((*it)->partOf != NULL)
    return it;

  auto declareToken = std::make_shared<Declare>();
  analyzedTokens.push_back(declareToken);
  it = omitTokens(it, {TokenType::HASH});

  it = createIdentifier(it, declareToken, {TokenType::ENDL, TokenType::END_OF_FILE});
  analyzedTokens.push_back(*it);

  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::createAssignment(Iterator it)
{
  it = rewindIterator(it, {TokenType::ENDL});

  auto startingIndex = findLastAnalyzedTokenIndex({TokenType::ENDL}) + 1;
  analyzedTokens.erase(analyzedTokens.begin() + startingIndex, analyzedTokens.end());

  auto assignToken = std::make_shared<Assign>();
  analyzedTokens.push_back(assignToken);
  it = createIdentifier(it, assignToken, {TokenType::EQUALS});

  auto withToken = std::make_shared<With>();
  analyzedTokens.push_back(withToken);
  it = createExpression(it, withToken, {TokenType::ENDL, TokenType::END_OF_FILE});
  analyzedTokens.push_back(*it);

  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::createIdentifier(
    Iterator it,
    std::shared_ptr<Token> partOf,
    std::vector<TokenType> end)
{
  auto identifierToken = std::make_shared<Identifier>();
  identifierToken->partOf = partOf;
  analyzedTokens.push_back(identifierToken);

  if ((*it)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected identifier");
  }

  while (!(*it)->isTypeOf(end))
  {
    if ((*it)->type == TokenType::END_OF_FILE)
    {
      throw SYNTAX_ERROR("Unexpected end of file.");
    }

    if ((*it)->type == TokenType::LBRACE)
    {
      it = createParameter(it, identifierToken);
      continue;
    }

    analyzedTokens.push_back(*it);
    analyzedTokens.back()->partOf = identifierToken;
    ++it;
  }

  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::createParameter(Iterator it, std::shared_ptr<Token> identifier)
{
  auto parameterToken = std::make_shared<Parameter>();
  parameterToken->partOf = identifier;
  analyzedTokens.push_back(parameterToken);
  ++it;

  while ((*it)->type != TokenType::RBRACE)
  {
    if ((*it)->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
    {
      throw SYNTAX_ERROR("Expected closing brace before end of line.");
    }

    analyzedTokens.push_back(*it);
    analyzedTokens.back()->partOf = parameterToken;
    ++it;
  }
  return ++it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::createExpression(Iterator it, std::shared_ptr<Token> partOf, std::vector<TokenType> end)
{
  auto expressionToken = std::make_shared<Expression>();
  expressionToken->partOf = partOf;
  analyzedTokens.push_back(expressionToken);

  if ((*it)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected expression");
  }

  while (!(*++it)->isTypeOf(end))
  {
    analyzedTokens.push_back(*it);
    analyzedTokens.back()->partOf = expressionToken;
  }
  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::searchIdentifier(Iterator it)
{
  auto possibleDefinitions = definitions;
  auto startingPoint = it;

  for (int i = possibleDefinitions.size() - 1; i >= 0; --i)
  {
    auto definition = definitions[i];
    auto reader = startingPoint;
    std::shared_ptr<Token> endToken;

    for (auto ident = definition->next; ident != NULL && ident->isPartOf({definition}); ident = ident->next)
    {
      if (ident->type == TokenType::PARAMETER)
      {
        ident = ident->next;

        while (!ident->isPartOf({definition}) && !ident->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
        {
          ident = ident->next;
        }

        if (ident->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
        {
          break;
        }
        endToken = ident;
        ++reader;

        while ((*reader)->literal != endToken->literal)
        {
          if ((*reader)->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
          {
            possibleDefinitions.erase(possibleDefinitions.begin() + i);
            break;
          }
          ++reader;
        }
      }

      if (ident->literal != (*reader)->literal)
      {
        possibleDefinitions.erase(possibleDefinitions.begin() + i);
        break;
      }
      ++reader;
    }
  }

  if (possibleDefinitions.size() > 0)
  {
    auto callToken = std::make_shared<Call>();
    auto definition = possibleDefinitions[0];

    callToken->relatedTo = definition;
    analyzedTokens.push_back(callToken);

    for (auto definitionToken = definition->next; !definitionToken->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE});)
    {

      if (definitionToken->type == TokenType::PARAMETER)
      {
        auto paramToken = definitionToken;
        definitionToken = definitionToken->next;
        while (!definitionToken->isPartOf({definition}) && !definitionToken->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
        {
          definitionToken = definitionToken->next;
        }

        it = createArgument(it, callToken, paramToken, definitionToken);
        continue;
      }
      analyzedTokens.push_back(*it);
      analyzedTokens.back()->partOf = callToken;
      ++it;
      definitionToken = definitionToken->next;
    }
  }
  else
  {
    throw SYNTAX_ERROR("Unknown word");
  }

  analyzedTokens.push_back(*it);

  return startingPoint + std::distance(startingPoint, it);
}

template <typename Iterator>
Iterator SemanticAnalyzer::createArgument(Iterator it,
                                          std::shared_ptr<Token> callToken,
                                          std::shared_ptr<Token> paramToken,
                                          std::shared_ptr<Token> endToken)
{
  auto argToken = std::make_shared<Argument>();
  argToken->partOf = callToken;
  argToken->relatedTo = paramToken;
  analyzedTokens.push_back(argToken);
  std::vector<TokenType> end = {endToken->type};

  if (endToken->type == TokenType::ENDL || endToken->type == TokenType::END_OF_FILE)
  {
    end = {TokenType::ENDL, TokenType::END_OF_FILE};
  }

  if ((*it)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected argument");
  }

  while (!(*it)->isTypeOf(end))
  {
    analyzedTokens.push_back(*it);
    analyzedTokens.back()->partOf = argToken;
    ++it;
  }

  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::omitTokens(Iterator it, std::vector<TokenType> types)
{
  while ((*it)->isTypeOf(types))
  {
    ++it;
  }
  return it;
}

template <typename Iterator>
Iterator SemanticAnalyzer::rewindIterator(Iterator it, std::vector<TokenType> types)
{
  while (!(*it)->isTypeOf(types))
  {
    --it;
    if (it == tokens.begin())
    {
      return it;
    }
  }

  return ++it;
}

int SemanticAnalyzer::findLastAnalyzedTokenIndex(std::vector<TokenType> types)
{
  auto it = analyzedTokens.end() - 1;
  while (!(*it)->isTypeOf(types))
  {
    if (it == analyzedTokens.begin())
      return -1;
    --it;
  }

  return it - analyzedTokens.begin();
}

std::string SemanticAnalyzer::verboseTokens()
{
  std::string verbal = "";
  for (auto token : analyzedTokens)
  {
    if (token->type == TokenType::ENDL && verbal.size() > 0)
    {
      verbal.pop_back();
    }

    verbal += token->verboseToken();
    if (token->type != TokenType::END_OF_FILE && token->type != TokenType::ENDL)
    {
      verbal += " ";
    }
  }
  verbal.pop_back();
  return verbal;
}
