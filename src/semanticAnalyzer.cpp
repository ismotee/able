#include "semanticAnalyzer.h"
#include "errors.h"
#include <iostream>

void SemanticAnalyzer::analyze()
{
  // put this in somewhere better place in the future
  keyphrases::prepare();

  handleDefinitions();
  prepareTokens();
  gatherDefinitions();
  handleCalls();
  prepareTokens();
  handleKeywords();
}

void SemanticAnalyzer::handleDefinitions()
{
  srcIt = tokens.begin();
  while (srcIt != tokens.end())
  {
    switch ((*srcIt)->type)
    {
    case TokenType::LBRACKET:
      createLink();
      break;
    case TokenType::HASH:
      createDeclaration();
      break;
    case TokenType::EQUALS:
      createAssignment();
      break;
    default:
      analyzedTokens.push_back(*srcIt);
      break;
    }
    ++srcIt;
  }
}

void SemanticAnalyzer::gatherDefinitions()
{
  srcIt = tokens.begin();
  while (srcIt != tokens.end())
  {
    if ((*srcIt)->type == TokenType::IDENTIFIER)
    {
      definitions.push_back(*srcIt);
    }
    ++srcIt;
  }
}

void SemanticAnalyzer::handleCalls()
{
  srcIt = tokens.begin();
  while (srcIt != tokens.end())
  {
    switch ((*srcIt)->type)
    {
    case TokenType::WORD:
    {
      if ((*srcIt)->partOf == NULL)
      {
        searchIdentifier();
      }
      else
      {
        analyzedTokens.push_back(*srcIt);
      }
      break;
    }
    default:
      analyzedTokens.push_back(*srcIt);
    }
    ++srcIt;
  }
}

void SemanticAnalyzer::handleKeywords()
{
  srcIt = tokens.begin();
  while (srcIt != tokens.end())
  {
    switch ((*srcIt)->type)
    {
    case TokenType::WORD:
    {
      if ((*srcIt)->partOf == nullptr)
      {
        searchKeyPhrase();
      }
      else
      {
        analyzedTokens.push_back(*srcIt);
      }
      break;
    }
    default:
      analyzedTokens.push_back(*srcIt);
    }
    ++srcIt;
  }
}

void SemanticAnalyzer::prepareTokens()
{
  tokens = analyzedTokens;
  analyzedTokens = {};

  token::linkTokens(tokens);
}

void SemanticAnalyzer::createLink()
{
  auto importToken = std::make_shared<Import>();
  analyzedTokens.push_back(importToken);
  ++srcIt;
  createIdentifier(importToken, {TokenType::RBRACKET});

  auto fromToken = std::make_shared<From>();
  analyzedTokens.push_back(fromToken);
  srcIt += 2;
  createIdentifier(fromToken, {TokenType::RBRACKET});
}

void SemanticAnalyzer::createDeclaration()
{
  if ((*srcIt)->partOf != NULL)
    return;

  auto declareToken = std::make_shared<Declare>();
  analyzedTokens.push_back(declareToken);
  omitTokens({TokenType::HASH});

  createIdentifier(declareToken, {TokenType::ENDL, TokenType::END_OF_FILE});
  analyzedTokens.push_back(*srcIt);
}

void SemanticAnalyzer::createAssignment()
{
  removeLastAnalyzedLine();

  auto assignToken = std::make_shared<Assign>();
  analyzedTokens.push_back(assignToken);
  createIdentifier(assignToken, {TokenType::EQUALS});

  auto withToken = std::make_shared<With>();
  analyzedTokens.push_back(withToken);
  createExpression(withToken, {TokenType::ENDL, TokenType::END_OF_FILE});
  analyzedTokens.push_back(*srcIt);
}

void SemanticAnalyzer::removeLastAnalyzedLine()
{
  rewindIterator({TokenType::ENDL});
  auto startingIndex = findLastAnalyzedTokenIndex({TokenType::ENDL}) + 1;
  analyzedTokens.erase(analyzedTokens.begin() + startingIndex, analyzedTokens.end());
}

void SemanticAnalyzer::createIdentifier(
    std::shared_ptr<Token> partOf,
    std::vector<TokenType> end)
{
  auto identifierToken = std::make_shared<Identifier>();
  identifierToken->partOf = partOf;
  analyzedTokens.push_back(identifierToken);

  if ((*srcIt)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected identifier");
  }

  while (!(*srcIt)->isTypeOf(end))
  {
    if ((*srcIt)->type == TokenType::END_OF_FILE)
    {
      throw SYNTAX_ERROR("Unexpected end of file.");
    }

    if ((*srcIt)->type == TokenType::LBRACE)
    {
      createParameter(identifierToken);
      continue;
    }

    analyzedTokens.push_back(*srcIt);
    analyzedTokens.back()->partOf = identifierToken;
    ++srcIt;
  }
}

void SemanticAnalyzer::createParameter(std::shared_ptr<Token> identifier)
{
  auto parameterToken = std::make_shared<Parameter>();
  parameterToken->partOf = identifier;
  analyzedTokens.push_back(parameterToken);
  ++srcIt;

  while ((*srcIt)->type != TokenType::RBRACE)
  {
    if ((*srcIt)->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
    {
      throw SYNTAX_ERROR("Expected closing brace before end of line.");
    }

    analyzedTokens.push_back(*srcIt);
    analyzedTokens.back()->partOf = parameterToken;
    ++srcIt;
  }
  ++srcIt;
}

void SemanticAnalyzer::createExpression(std::shared_ptr<Token> partOf, std::vector<TokenType> end)
{
  auto expressionToken = std::make_shared<Expression>();
  expressionToken->partOf = partOf;
  analyzedTokens.push_back(expressionToken);

  if ((*srcIt)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected expression");
  }

  while (!(*++srcIt)->isTypeOf(end))
  {
    analyzedTokens.push_back(*srcIt);
    analyzedTokens.back()->partOf = expressionToken;
  }
}

void SemanticAnalyzer::searchIdentifier()
{
  auto possibleDefinitions = definitions;
  auto startingPoint = srcIt;

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

        createArgument(callToken, paramToken, definitionToken);
        continue;
      }
      analyzedTokens.push_back(*srcIt);
      analyzedTokens.back()->partOf = callToken;
      ++srcIt;
      definitionToken = definitionToken->next;
    }
  }
  else
  {
    analyzedTokens.push_back(*srcIt);
    return;
  }

  analyzedTokens.push_back(*srcIt);
  return;
}

void SemanticAnalyzer::searchKeyPhrase()
{
  // this one is tricky.
  // First of all source needs to be compared to key phrases.
  // After the match is found there should be some kind of argument harvesting.
  // Lastly key phrase call will be created from the pregenerated keyphrase list.
  // Every step here is somewhat complex.

  std::string phraseName;

  // find a key phrase from pre-generated definitions
  for (auto &keyphrase : keyphrases::builtinKeyPhrases)
  {
    // assuming that every keyphrase has at least one token
    auto curPToken = keyphrase.second.at(0);
    auto curSToken = (*srcIt);
    // This is used as a indicator
    phraseName = keyphrase.first;

    while (curPToken != nullptr && curSToken != nullptr)
    {
      // there's three cases in matching:
      // 1) Tokens match. check next token.
      // 2a) Key phrase has a parameter. Parameter is at the end of the phrase.
      //     If so, then it's a match.
      // 2b) Key phrase has a parameter. Parameter is at the middle of the phrase.
      //     If so, then source needs to be fast forwarded and checked if there's
      //     matching token to end the parameter. If so, check next token.
      // 3) Otherwise it not a match.

      if (curPToken->type == curSToken->type && curPToken->literal == curSToken->literal)
      {
        curSToken = curSToken->next;
        curPToken = curPToken->next;
      }
      else if (curPToken->type == TokenType::PARAMETER)
      {
        // assuming parameter name to be exactly length of one token.
        // this is merely a way to make this part of the code a way simpler.
        curPToken = curPToken->next;
        curPToken = curPToken->next;

        if (curPToken == nullptr)
        {
          // whatever comes after, it is considered as an argument
          // therefore it is possible to assume that this is a matching keyphrase
          phraseName = keyphrase.first;
          break;
        }
        else
        {
          auto paramEnd = curPToken;

          while (curSToken->type != TokenType::ENDL && curSToken->type != TokenType::END_OF_FILE && curSToken->type == paramEnd->type && curSToken->literal == paramEnd->literal)
          {
            curSToken = curSToken->next;
          }

          if (curSToken->type != TokenType::ENDL && curSToken->type != TokenType::END_OF_FILE)
          {
            // most likely this will cause an error later on but there's not enough
            // information to throw an error at the moment
            break;
          }
        }

        curPToken = curPToken->next;
        curSToken = curSToken->next;
      }
      else
      {
        phraseName = "";
        break;
      }
    }

    // match found
    if (phraseName.length() > 0)
    {
      break;
    }
  }

  // no match found. Add source token to processed tokens as is
  // and let the error handling take of it later on.
  if (phraseName.length() == 0)
  {
    analyzedTokens.push_back(*srcIt);
    return;
  }

  // get arguments
  auto keyphrase = keyphrases::builtinKeyPhrases[phraseName];
  auto pToken = keyphrase[0];
  auto sToken = (*srcIt);
  std::vector<Tokens> args;
  Tokens paramNames = {};

  while (pToken != nullptr)
  {
    if (pToken->isTypeOf({TokenType::PARAMETER}))
    {
      Tokens arg = {};
      pToken = pToken->next;
      paramNames.push_back(pToken);
      pToken = pToken->next;
      std::shared_ptr<Token> endToken = pToken;

      if (endToken == nullptr)
      {
        while (!sToken->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
        {
          arg.push_back(sToken);
          sToken = sToken->next;
        }
      }
      else
      {
        while (sToken != nullptr && sToken->isTypeOf({endToken->type}) && sToken->literal != endToken->literal)
        {
          // at this point there should not need end of file or endl checks because
          // matching phrase is already found.
          arg.push_back(sToken);
          sToken->next;
        }
      }
      args.push_back(arg);
    }
    else
    {
      pToken = pToken->next;
      sToken = sToken->next;
    }
  }

  // generate a key phrase call
  auto cToken = keyphrases::builtinPhraseCalls[phraseName].at(0);
  u_int curArg = 0;
  u_int callLength = 0;

  while (cToken != nullptr)
  {
    if (cToken->isTypeOf({TokenType::ARGUMENT}))
    {
      auto argToken = std::make_shared<Argument>();
      auto exprToken = std::make_shared<Expression>(argToken);
      analyzedTokens.push_back(argToken);
      analyzedTokens.push_back(exprToken);

      for (auto it = args[curArg].begin(); it != args[curArg].end(); ++it)
      {
        (*it)->partOf = exprToken;
        (*it)->relatedTo = paramNames[curArg];
        analyzedTokens.push_back(*it);
        ++callLength;
      }
    }
    else
    {
      analyzedTokens.push_back(cToken);
      ++callLength;
    }

    if (cToken != nullptr)
    {
      cToken = cToken->next;
    }
  }

  srcIt += callLength;
  return;
}

void SemanticAnalyzer::createArgument(
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

  if ((*srcIt)->isTypeOf(end))
  {
    throw SYNTAX_ERROR("Expected argument");
  }

  while (!(*srcIt)->isTypeOf(end))
  {
    analyzedTokens.push_back(*srcIt);
    analyzedTokens.back()->partOf = argToken;
    ++srcIt;
  }
}

void SemanticAnalyzer::omitTokens(std::vector<TokenType> types)
{
  while (*srcIt != nullptr && (*srcIt)->isTypeOf(types))
  {
    ++srcIt;
  }
}

void SemanticAnalyzer::rewindIterator(std::vector<TokenType> types)
{
  while (!(*srcIt)->isTypeOf(types))
  {
    --srcIt;
    if (srcIt == tokens.begin())
    {
      return;
    }
  }
  ++srcIt;
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
