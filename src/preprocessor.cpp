#include "preprocessor.h"
#include <algorithm>
#include <iostream>

bool PreProcessor::separateLine(Tokens& t, Tokens::iterator& begin, std::vector<TokenType> end) {
    line = {};
    auto endIt = std::find_if(begin, t.end(), [end](pToken token) {return token->isTypeOf(end); });
    line.assign(begin, endIt);

    return endIt != t.end();
}


bool DefinitionProcessor::check(Tokens& t, Tokens::iterator it) {
    separateLine(t, it);

    if (line.size() == 0) {
        return false;
    }

    if (!(*line.begin())->isTypeOf(TokenType::HASH)) {
        return false;
    }

    return true;
}

std::shared_ptr<PreProcessorError> DefinitionProcessor::verify(Tokens& t, Tokens::iterator it) {
    if (!(*it)->isTypeOf(TokenType::HASH)) {
        return std::make_shared<PreProcessorError>("Definition must start with #");
    }

    while ((*it)->isTypeOf(TokenType::HASH)) {
        it = std::next(it);
    }

    IdentifierProcessor idProcessor;
    auto error = idProcessor.verify(t, it);
    if (error) {
        return error;
    }

    return nullptr;
}

std::shared_ptr<PreToken> DefinitionProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto token = std::make_shared<PreDeclare>();
    auto scope = std::make_shared<PreBlock>();

    while ((*it)->isTypeOf(TokenType::HASH)) {
        ++scope->depth;
        it = std::next(it);
    }

    separateLine(t, it);

    IdentifierProcessor idProcessor;
    token->tokens.push_back(idProcessor.create(t, it));
    token->tokens.push_back(scope);
    return token;
}


bool IdentifierProcessor::check(Tokens& t, Tokens::iterator it) {
    separateLine(t, it);
    return line.size() != 0;
}


std::shared_ptr<PreProcessorError> IdentifierProcessor::verify(Tokens& t, Tokens::iterator it) {
    if (!check(t, it)) {
        return std::make_shared<PreProcessorError>("Identifier cannot be blank");
    }

    auto hasIlligalToken = std::find_if(line.begin(), line.end(), [](pToken token) {
        return token->isTypeOf({
            TokenType::EQUALS, TokenType::EQUALS_COMPARE,
            TokenType::PLUS, TokenType::MINUS,
            TokenType::SLASH, TokenType::ASTERISK,
            TokenType::COLON, TokenType::BANG,
            TokenType::GT, TokenType::GT_OR_EQUALS,
            TokenType::LT, TokenType::LT_OR_EQUALS,
            TokenType::NOT_EQUALS, TokenType::HASH,
            TokenType::ENDL, TokenType::END_OF_FILE,
            TokenType::NUMBER });
        });

    if (hasIlligalToken != line.end()) {
        return std::make_shared<PreProcessorError>("Identifier has an illigal token: " + (*hasIlligalToken)->literal);
    }

    return nullptr;
}

std::shared_ptr<PreToken> IdentifierProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto token = std::make_shared<PreIdentifier>();
    while (!(*it)->isTypeOf(token::END)) {
        if ((*it)->isTypeOf(TokenType::LBRACE)) {
            ParameterProcessor pp;
            token->tokens.push_back(pp.create(t, it));
        } else {
            token->tokens.push_back(*it);
        }
        it = std::next(it);
    }
    return token;
}


bool ParameterProcessor::check(Tokens& t, Tokens::iterator it) {
    return (*it)->isTypeOf(TokenType::LBRACE);
}

std::shared_ptr<PreProcessorError> ParameterProcessor::verify(Tokens& t, Tokens::iterator it) {
    if (!check(t, it)) {
        return std::make_shared<PreProcessorError>("Parameter doesn't start with open brace");
    }

    auto hasLine = separateLine(t, it, { TokenType::RBRACE });
    if (!hasLine) {
        return std::make_shared<PreProcessorError>("Parameter missing enclosing brace");
    }

    IdentifierProcessor ip;
    return ip.verify(t, it);
}

std::shared_ptr<PreToken> ParameterProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto token = std::shared_ptr<PreParameter>();
    IdentifierProcessor ip;
    token->tokens.push_back(ip.create(t, it));
    return token;
}