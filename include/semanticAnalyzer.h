#pragma once
#include "token.h"
#include "scope.h"
#include "keyphrases.h"
#include <unordered_map>

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
  void handleKeyPhrases();
  void prepareTokens();
  void finalize();

  // sub-methods
  void createKeyPhrase();
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
  std::string searchKeyPhrase();
  std::vector<Tokens> gatherKeyPhraseArgs(std::string keyPhraseName);
  void addKeyPhrase(std::string keyPhraseName, std::vector<Tokens> args);
  void omitTokens(std::vector<TokenType> types);
  void rewindIterator(std::vector<TokenType> types);
  void removeLastAnalyzedLine();
  int findLastAnalyzedTokenIndex(std::vector<TokenType> types);

private:
  Tokens::iterator srcIt;
};
