#pragma once
#include "token.h"
#include "ast.h"
#include "preprocessor.h"
#include <iterator>

class PreParser;


class PreParser {
public:
    PreParser(Tokens t = {}): tokens(t) {
        processors.push_back(std::make_shared<DefinitionProcessor>());
    }

    void prepareFromStart();
    void prepare(Tokens::iterator it);

    Tokens tokens;
    Tokens parsedTokens;
    std::vector<std::shared_ptr<PreProcessor>> processors;

};

