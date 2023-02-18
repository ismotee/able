#pragma once
#include "token.h"
#include "ast.h"
#include "preprocessor.h"
#include "scopeBuilder.h"
#include <iterator>

class PreParser;

class PreParser {
public:
    PreParser(Tokens t = {}): tokens(t), parsedTokens({}), scoped(nullptr) {
        processors.push_back(std::make_shared<DefinitionProcessor>());
        processors.push_back(std::make_shared<AssignmentProcessor>());
        processors.push_back(std::make_shared<ExpressionStatementProcessor>());
    }

    void prepareFromStart();
    void prepare(Tokens::iterator it);
    void prepareStatements(Tokens::iterator& it);
    void flattenDeclarations();
    void prepareScopeTree(Tokens::iterator it);
    void gatherIdentifiers();
    void gatherIdentifiersForScope(pPreScope scope);

    Tokens tokens;
    Tokens parsedTokens;
    pPreScope scoped;
    std::vector<std::shared_ptr<PreProcessor>> processors;
};
