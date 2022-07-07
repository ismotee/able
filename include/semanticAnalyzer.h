#pragma once
#include "token.h"
#include "scope.h"
#include "keyphrases.h"

class SemanticAnalyzer
{
public:
  SemanticAnalyzer() {}
  SemanticAnalyzer(Tokens t) : tokens(t)
  {
    analyze();
  }

  void analyze();
  std::string verboseTokens();

  Tokens tokens;
  Tokens analyzedTokens;
  Tokens definitions;
  ptrScope root;
  ptrScope current;

protected:
  void handleDefinitions();
  void gatherDefinitions();
  void handleCalls();
  void handleKeywords();
  void prepareTokens();

  // sub-methods
  void createLink();
  void createDeclaration();
  void createAssignment();
  void createIdentifier(std::shared_ptr<Token> partOf, std::vector<TokenType> end);
  void createParameter(std::shared_ptr<Token> identifier);
  void createExpression(std::shared_ptr<Token> partOf, std::vector<TokenType> end);
  void createArgument(
      std::shared_ptr<Token> callToken,
      std::shared_ptr<Token> paramToken,
      std::shared_ptr<Token> endToken);
  void searchIdentifier();
  void searchKeyPhrase();
  void omitTokens(std::vector<TokenType> types);
  void rewindIterator(std::vector<TokenType> types);
  void removeLastAnalyzedLine();
  int findLastAnalyzedTokenIndex(std::vector<TokenType> types);

private:
  Tokens::iterator srcIt;
};
