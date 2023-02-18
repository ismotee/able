#include "preprocessor.h"
#include <algorithm>
#include <iostream>

bool PreProcessor::separateLine(Tokens& t, Tokens::iterator& begin, std::vector<TokenType> end) {
    line = {};
    auto endIt = std::find_if(begin, t.end(), [end](pToken token) { return token->isTypeOf(end); });
    line.assign(begin, endIt);

    return endIt != t.end();
}

bool DefinitionProcessor::check(Tokens& t, Tokens::iterator it) {
    return (*it)->isTypeOf(TokenType::HASH);
}

std::shared_ptr<PreProcessorError> DefinitionProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto hashCount = preprocessor::expectMultiple(it, { TokenType::HASH });

    if (hashCount == -1) {
        return std::make_shared<PreProcessorError>("Definition: hash symbols not found.");
    }

    if (hashCount == 0) {
        return std::make_shared<PreProcessorError>("Definition must start with #");
    }

    DeclarationIdentifierProcessor idProcessor;
    auto error = idProcessor.verify(t, it);
    if (error) {
        error->msg = "Definition: " + error->msg;
        return error;
    }
    return nullptr;
}

std::shared_ptr<PreToken> DefinitionProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto token = std::make_shared<PreDeclare>();
    auto scope = std::make_shared<PreBlock>();

    auto depth = preprocessor::expectMultiple(it, { TokenType::HASH });
    token->depth = depth;

    DeclarationIdentifierProcessor idProcessor;
    token->tokens.push_back(idProcessor.create(t, it));
    return token;
}

bool AssignmentProcessor::check(Tokens& t, Tokens::iterator it) {
    auto count = preprocessor::fastForwardUntil(it, { TokenType::EQUALS });
    return count != -1;
}

std::shared_ptr<PreProcessorError> AssignmentProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto ip = DeclarationIdentifierProcessor();
    ip.endsWith = { TokenType::EQUALS };
    auto idError = ip.verify(t, it);

    if (idError) {
        return idError;
    }

    it = t.begin() + ip.endPosition;

    if (!(*it)->isTypeOf({ TokenType::EQUALS })) {
        return preprocessor::error("Assignment: Identifier should end with Equals sign");
    }

    it = std::next(it);

    if ((*it)->isTypeOf(token::END)) {
        return preprocessor::error("Assignment: Cannot assign empty");
    }

    auto ep = ExpressionProcessor();
    auto expError = ep.verify(t, it);

    if (expError) {
        return expError;
    }

    it = t.begin() + ep.endPosition;
    return nullptr;
}

std::shared_ptr<PreToken> AssignmentProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto assignToken = std::make_shared<PreAssignment>();

    auto ip = DeclarationIdentifierProcessor();
    ip.setEndsWith({ TokenType::EQUALS });
    auto idToken = ip.create(t, it);

    // equals sign
    it = std::next(it);

    auto ep = ExpressionProcessor();
    auto expressionToken = ep.create(t, it);

    assignToken->tokens.push_back(idToken);
    assignToken->tokens.push_back(expressionToken);
    return assignToken;
}

bool ImportProcessor::check(Tokens& t, Tokens::iterator it) {
    if (!(*it)->isTypeOf(TokenType::LBRACKET)) {
        return false;
    }

    if (preprocessor::fastForwardUntil(it, { TokenType::RBRACKET }) == -1) {
        return false;
    }

    return preprocessor::expect(it, { TokenType::LBRACE });
}

std::shared_ptr<PreProcessorError> ImportProcessor::verify(Tokens& t, Tokens::iterator it) {
    if (!(*it)->isTypeOf(TokenType::LBRACKET)) {
        return preprocessor::error("Import: Should start with [");
    }

    if (preprocessor::fastForwardUntil(it, { TokenType::RBRACKET }) == -1) {
        return preprocessor::error("Import: Closing bracket is missing.");
    }

    if (!preprocessor::expect(it, { TokenType::LBRACE })) {
        return preprocessor::error("Import: Unexpected token after brackets: " + (*it)->literal);
    }

    it = std::next(it);
    auto linkCount = preprocessor::fastForwardUntil(it, { TokenType::RBRACE });

    if (linkCount == -1) {
        return preprocessor::error("Import: Closing brace is missing.");
    }

    if (linkCount == 0) {
        return preprocessor::error("Import: Link should not be empty");
    }

    endPosition = std::distance(t.begin(), it);
    return nullptr;
}

std::shared_ptr<PreToken> ImportProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto importToken = std::make_shared<PreImport>();

    preprocessor::expect(it, { TokenType::LBRACKET });

    while (!(*it)->isTypeOf(TokenType::RBRACKET)) {
        it = std::next(it);
        // bypass description for now. It would be convenient to have access in import description later on.
    }

    preprocessor::expect(it, { TokenType::LBRACE });
    it = std::next(it);

    while (!(*it)->isTypeOf(TokenType::RBRACE)) {
        importToken->tokens.push_back(*it);
        it = std::next(it);
    }

    return importToken;
}

bool ExpressionStatementProcessor::check(Tokens& t, Tokens::iterator it) {
    auto ep = ExpressionProcessor();
    return ep.check(t, it);
}

std::shared_ptr<PreProcessorError> ExpressionStatementProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto ep = ExpressionProcessor();
    return ep.verify(t, it);
}

std::shared_ptr<PreToken> ExpressionStatementProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto expressionStatement = std::make_shared<PreExpressionStatement>();
    auto ep = ExpressionProcessor();
    expressionStatement->tokens.push_back(ep.create(t, it));
    return expressionStatement;
}

bool ExpressionProcessor::check(Tokens& t, Tokens::iterator it) {
    if ((*it)->isTypeOf(legalTokens)) {
        return true;
    }
    return false;
}

std::shared_ptr<PreProcessorError> ExpressionProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto ip = ImportProcessor();
    auto ident = IdentifierProcessor();

    while (!(*it)->isTypeOf(endsWith)) {
        if (!check(t, it)) {
            return std::make_shared<PreProcessorError>("Expression: Illigal symbol: " + (*it)->literal);
        }

        if (ident.check(t, it)) {
            auto error = ident.verify(t, it);
            if (error != nullptr) {
                error->msg = "Expression: " + error->msg;
                return error;
            }
            it = t.begin() + ident.endPosition;
            continue;
        }

        if (ip.check(t, it)) {
            auto error = ip.verify(t, it);
            if (error != nullptr) {
                error->msg = "Expression: " + error->msg;
                return error;
            }
            it = t.begin() + ip.endPosition;
            continue;
        }
        it = std::next(it);
    }

    endPosition = std::distance(t.begin(), it);
    return nullptr;
}

std::shared_ptr<PreToken> ExpressionProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto et = std::make_shared<PreExpression>();
    auto ip = ImportProcessor();
    auto ident = IdentifierProcessor();

    while (!(*it)->isTypeOf(endsWith)) {
        if (ip.check(t, it)) {
            et->tokens.push_back(ip.create(t, it));
        } else if (ident.check(t, it)) {
            et->tokens.push_back(ident.create(t, it));
            continue;
        } else {
            et->tokens.push_back(*it);
        }

        it = std::next(it);
    }
    return et;
}

bool IdentifierProcessor::check(Tokens& t, Tokens::iterator it) {
    return (*it)->isTypeOf({ TokenType::WORD, TokenType::UNDERSCORE });
}

std::shared_ptr<PreProcessorError> IdentifierProcessor::verifyIdentifier(Tokens& t, Tokens::iterator it, std::shared_ptr<ParameterProcessor> pp) {
    while (!(*it)->isTypeOf(endsWith)) {
        if (!check(t, it)) {
            return preprocessor::error("unexpected token: " + (*it)->typeToString());
        }

        if (pp->check(t, it)) {
            auto error = pp->verify(t, it);

            if (error) {
                error->msg = "Parameter: " + error->msg;
                return error;
            }

            it = t.begin() + pp->endPosition;
        }

        it = std::next(it);
    }

    endPosition = std::distance(t.begin(), it);
    return nullptr;
}

std::shared_ptr<PreToken> IdentifierProcessor::createIdentifier(Tokens& t, Tokens::iterator& it, std::shared_ptr<ParameterProcessor> pp) {
    auto token = std::make_shared<PreIdentifier>();

    while (!(*it)->isTypeOf(endsWith)) {
        if (pp->check(t, it)) {
            token->tokens.push_back(pp->create(t, it));
        } else {
            token->tokens.push_back(*it);
        }
        it = std::next(it);
    }
    return token;
}

std::shared_ptr<PreProcessorError> IdentifierProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto pp = std::make_shared<ActualParameterProcessor>();
    return verifyIdentifier(t, it, pp);
}

std::shared_ptr<PreToken> IdentifierProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto pp = std::make_shared<ActualParameterProcessor>();
    return createIdentifier(t, it, pp);
}


std::shared_ptr<PreProcessorError> DeclarationIdentifierProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto pp = std::make_shared<FormalParameterProcessor>();
    return verifyIdentifier(t, it, pp);
}

std::shared_ptr<PreToken> DeclarationIdentifierProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto pp = std::make_shared<FormalParameterProcessor>();
    return createIdentifier(t, it, pp);
}


bool ParameterProcessor::check(Tokens& t, Tokens::iterator it) {
    return (*it)->isTypeOf(TokenType::UNDERSCORE);
}

std::shared_ptr<PreProcessorError> ParameterProcessor::verifyParameter(Tokens& t, Tokens::iterator& it) {
    if (!check(t, it)) {
        return preprocessor::error("Parameter: Should start with underscore");
    }

    it = std::next(it);

    if ((*it)->isTypeOf(TokenType::UNDERSCORE)) {
        return preprocessor::error("Parameter: parameter is missing");
    }

    while (!(*it)->isTypeOf(TokenType::UNDERSCORE)) {
        if ((*it)->isTypeOf(token::END)) {
            return preprocessor::error("Parameter: enclosing underscore is missing");
        }

        it = std::next(it);
    }

    endPosition = std::distance(t.begin(), it);
    return nullptr;
}

std::shared_ptr<PreToken> ParameterProcessor::createParameter(Tokens& t, Tokens::iterator& it, std::shared_ptr<ComponentProcessor> cp) {
    it = std::next(it);
    auto token = std::make_shared<PreParameter>();

    cp->endsWith = { TokenType::UNDERSCORE };

    token->tokens.push_back(cp->create(t, it));
    endPosition = std::distance(t.begin(), it);
    return token;
}

std::shared_ptr<PreProcessorError> ActualParameterProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto error = verifyParameter(t, it);
    if (error) {
        return error;
    }

    auto ep = ExpressionProcessor();
    ep.endsWith = { TokenType::UNDERSCORE };
    return ep.verify(t, it);
}

std::shared_ptr<PreToken> ActualParameterProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto ep = std::make_shared<ExpressionProcessor>();
    return createParameter(t, it, ep);
}

std::shared_ptr<PreProcessorError> FormalParameterProcessor::verify(Tokens& t, Tokens::iterator it) {
    auto error = verifyParameter(t, it);
    if (error) {
        return error;
    }

    IdentifierProcessor ip;
    ip.endsWith = { TokenType::UNDERSCORE };
    return ip.verify(t, it);
}

std::shared_ptr<PreToken> FormalParameterProcessor::create(Tokens& t, Tokens::iterator& it) {
    auto ip = std::make_shared<IdentifierProcessor>();
    ip->endsWith = { TokenType::UNDERSCORE };
    return createParameter(t, it, ip);
}

