#pragma once
#include "token.h"
#include "scope.h"

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

  template <typename Iterator>
  Iterator createLink(Iterator it);

  template <typename Iterator>
  Iterator createDeclaration(Iterator it);

  template <typename Iterator>
  Iterator createAssignment(Iterator it);

  template <typename Iterator>
  Iterator createIdentifier(Iterator it, std::shared_ptr<Token> partOf, std::vector<TokenType> end);

  template <typename Iterator>
  Iterator createParameter(Iterator it, std::shared_ptr<Token> identifier);

  template <typename Iterator>
  Iterator createExpression(Iterator it, std::shared_ptr<Token> partOf, std::vector<TokenType> end);

  template <typename Iterator>
  Iterator createArgument(Iterator it,
                          std::shared_ptr<Token> callToken,
                          std::shared_ptr<Token> paramToken,
                          std::shared_ptr<Token> endToken);

  template <typename Iterator>
  Iterator searchIdentifier(Iterator it);

  template <typename Iterator>
  Iterator omitTokens(Iterator it, std::vector<TokenType> types);

  template <typename Iterator>
  Iterator rewindIterator(Iterator it, std::vector<TokenType> types);

  int findLastAnalyzedTokenIndex(std::vector<TokenType> types);
};
