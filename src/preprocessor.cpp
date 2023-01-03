#include "preprocessor.h"
#include <algorithm>
#include <iostream>

bool PreProcessor::separateLine(Tokens &t, Tokens::iterator &begin, std::vector<TokenType> end)
{
    line = {};
    auto endIt = std::find_if(begin, t.end(), [end](pToken token)
                              { return token->isTypeOf(end); });
    line.assign(begin, endIt);

    return endIt != t.end();
}

bool DefinitionProcessor::check(Tokens &t, Tokens::iterator it)
{
    return (*it)->isTypeOf(TokenType::HASH);
}

std::shared_ptr<PreProcessorError> DefinitionProcessor::verify(Tokens &t, Tokens::iterator it)
{
    auto hashCount = preprocessor::expectMultiple(it, {TokenType::HASH});

    if (hashCount == -1)
    {
        return std::make_shared<PreProcessorError>("Definition: hash symbols not found.");
    }

    if (hashCount == 0)
    {
        return std::make_shared<PreProcessorError>("Definition must start with #");
    }

    IdentifierProcessor idProcessor;
    auto error = idProcessor.verify(t, it);
    if (error)
    {
        return error;
    }

    return nullptr;
}

std::shared_ptr<PreToken> DefinitionProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto token = std::make_shared<PreDeclare>();
    auto scope = std::make_shared<PreBlock>();

    auto depth = preprocessor::expectMultiple(it, {TokenType::HASH});
    token->depth = depth;

    separateLine(t, it);

    IdentifierProcessor idProcessor;
    token->tokens.push_back(idProcessor.create(t, it));
    return token;
}

bool AssignmentProcessor::check(Tokens &t, Tokens::iterator it)
{
    auto count = preprocessor::fastForwardUntil(it, {TokenType::EQUALS});
    return count != -1;
}

std::shared_ptr<PreProcessorError> AssignmentProcessor::verify(Tokens &t, Tokens::iterator it)
{
    IdentifierProcessor ip = IdentifierProcessor();
    ip.endsWith = {TokenType::EQUALS};
    auto idError = ip.verify(t, it);

    if (idError != nullptr)
    {
        return idError;
    }

    auto pos = it + ip.getLineLength();

    if (!(*pos)->isTypeOf({TokenType::EQUALS}))
    {
        return std::make_shared<PreProcessorError>("Assignment: Identifier should end with Equals sign");
    }

    pos = std::next(pos);

    if ((*it)->isTypeOf(token::END))
    {
        return std::make_shared<PreProcessorError>("Assignment: Cannot assign empty");
    }

    ExpressionProcessor ep = ExpressionProcessor();
    ep.verify(t, pos);

    return nullptr;
}

std::shared_ptr<PreToken> AssignmentProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto assignToken = std::make_shared<PreAssignment>();

    auto ip = IdentifierProcessor();
    ip.endsWith = {TokenType::EQUALS};
    auto idToken = ip.create(t, it);

    // equals sign
    it = std::next(it);

    auto ep = ExpressionProcessor();
    auto expressionToken = ep.create(t, it);

    assignToken->tokens.push_back(idToken);
    assignToken->tokens.push_back(expressionToken);
    return assignToken;
}

bool ImportProcessor::check(Tokens &t, Tokens::iterator it)
{
    if (!(*it)->isTypeOf(TokenType::LBRACKET))
    {
        return false;
    }

    if (preprocessor::fastForwardUntil(it, {TokenType::RBRACKET}) == -1)
    {
        return false;
    }

    return preprocessor::expect(it, {TokenType::LBRACE});
}

std::shared_ptr<PreProcessorError> ImportProcessor::verify(Tokens &t, Tokens::iterator it)
{
    if (!(*it)->isTypeOf(TokenType::LBRACKET))
    {
        return preprocessor::error("Import: Should start with [");
    }

    if (preprocessor::fastForwardUntil(it, {TokenType::RBRACKET}) == -1)
    {
        return preprocessor::error("Import: Closing bracket is missing.");
    }

    if (!preprocessor::expect(it, {TokenType::LBRACE}))
    {
        return preprocessor::error("Import: Unexpected token after brackets: " + (*it)->literal);
    }

    it = std::next(it);
    auto linkCount = preprocessor::fastForwardUntil(it, {TokenType::RBRACE});

    if (linkCount == -1)
    {
        return preprocessor::error("Import: Closing brace is missing.");
    }

    if (linkCount == 0)
    {
        return preprocessor::error("Import: Link should not be empty");
    }

    return nullptr;
}

std::shared_ptr<PreToken> ImportProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto importToken = std::make_shared<PreImport>();

    preprocessor::expect(it, {TokenType::LBRACKET});

    while (!(*it)->isTypeOf(TokenType::RBRACKET))
    {
        it = std::next(it);
        // bypass description for now. It would be convenient to have access in import description later on.
    }

    preprocessor::expect(it, {TokenType::LBRACE});
    it = std::next(it);

    while (!(*it)->isTypeOf(TokenType::RBRACE))
    {
        importToken->tokens.push_back(*it);
        it = std::next(it);
    }

    return importToken;
}

bool ExpressionStatementProcessor::check(Tokens &t, Tokens::iterator it)
{
    auto ep = ExpressionProcessor();
    return ep.check(t, it);
}

std::shared_ptr<PreProcessorError> ExpressionStatementProcessor::verify(Tokens &t, Tokens::iterator it)
{
    auto ep = ExpressionProcessor();
    return ep.verify(t, it);
}

std::shared_ptr<PreToken> ExpressionStatementProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto expressionStatement = std::make_shared<PreExpressionStatement>();
    auto ep = ExpressionProcessor();
    expressionStatement->tokens.push_back(ep.create(t, it));
    return expressionStatement;
}

bool ExpressionProcessor::check(Tokens &t, Tokens::iterator it)
{
    if ((*it)->isTypeOf({TokenType::EQUALS_COMPARE, TokenType::WORD,
                         TokenType::PLUS, TokenType::MINUS,
                         TokenType::SLASH, TokenType::ASTERISK,
                         TokenType::COLON, TokenType::BANG,
                         TokenType::LBRACE, TokenType::RBRACE,
                         TokenType::LBRACKET, TokenType::RBRACKET,
                         TokenType::GT, TokenType::GT_OR_EQUALS,
                         TokenType::LT, TokenType::LT_OR_EQUALS,
                         TokenType::NOT_EQUALS, TokenType::NUMBER,
                         TokenType::ENDL, TokenType::END_OF_FILE}))
    {
        return true;
    }

    return false;
}

std::shared_ptr<PreProcessorError> ExpressionProcessor::verify(Tokens &t, Tokens::iterator it)
{
    separateLine(t, it);
    auto ip = ImportProcessor();

    for (auto token : line)
    {
        if (!check(t, it))
        {
            return std::make_shared<PreProcessorError>("Expression: Illigal symbol: " + token->literal);
        }
        if (ip.check(t, it))
        {
            auto error = ip.verify(t, it);
            if (error != nullptr)
            {
                error->msg = "Expression: " + error->msg;
                return error;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<PreToken> ExpressionProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto et = std::make_shared<PreExpression>();
    auto ip = ImportProcessor();

    while (!(*it)->isTypeOf(token::END))
    {
        if (ip.check(t, it))
        {
            et->tokens.push_back(ip.create(t, it));
        }
        else
        {
            et->tokens.push_back(*it);
        }

        it = std::next(it);
    }
    return et;
}

bool IdentifierProcessor::check(Tokens &t, Tokens::iterator it)
{
    separateLine(t, it, endsWith);
    return line.size() != 0;
}

std::shared_ptr<PreProcessorError> IdentifierProcessor::verify(Tokens &t, Tokens::iterator it)
{
    if (!check(t, it))
    {
        return std::make_shared<PreProcessorError>("Identifier: should not be blank");
    }

    auto hasIlligalToken = std::find_if(line.begin(), line.end(), [](pToken token)
                                        { return token->isTypeOf({TokenType::EQUALS, TokenType::EQUALS_COMPARE,
                                                                  TokenType::PLUS, TokenType::MINUS,
                                                                  TokenType::SLASH, TokenType::ASTERISK,
                                                                  TokenType::COLON, TokenType::BANG,
                                                                  TokenType::GT, TokenType::GT_OR_EQUALS,
                                                                  TokenType::LT, TokenType::LT_OR_EQUALS,
                                                                  TokenType::NOT_EQUALS, TokenType::HASH,
                                                                  TokenType::ENDL, TokenType::END_OF_FILE,
                                                                  TokenType::NUMBER}); });

    if (hasIlligalToken != line.end())
    {
        return std::make_shared<PreProcessorError>("Identifier: illigal token: " + (*hasIlligalToken)->literal);
    }

    return nullptr;
}

std::shared_ptr<PreToken> IdentifierProcessor::create(Tokens &t, Tokens::iterator &it)
{
    auto token = std::make_shared<PreIdentifier>();
    while (!(*it)->isTypeOf(endsWith))
    {
        if ((*it)->isTypeOf(TokenType::LBRACE))
        {
            ParameterProcessor pp;
            token->tokens.push_back(pp.create(t, it));
        }
        else
        {
            token->tokens.push_back(*it);
        }
        it = std::next(it);
    }
    return token;
}

bool ParameterProcessor::check(Tokens &t, Tokens::iterator it)
{
    return (*it)->isTypeOf(TokenType::LBRACE);
}

std::shared_ptr<PreProcessorError> ParameterProcessor::verify(Tokens &t, Tokens::iterator it)
{
    if (!check(t, it))
    {
        return std::make_shared<PreProcessorError>("Parameter: Should start with open brace");
    }

    auto hasLine = separateLine(t, it, {TokenType::RBRACE});
    if (!hasLine)
    {
        return std::make_shared<PreProcessorError>("Parameter: missing enclosing brace");
    }

    IdentifierProcessor ip;
    return ip.verify(t, it);
}

std::shared_ptr<PreToken> ParameterProcessor::create(Tokens &t, Tokens::iterator &it)
{
    it = std::next(it);
    auto token = std::make_shared<PreParameter>();

    IdentifierProcessor ip = IdentifierProcessor();
    ip.endsWith = {TokenType::RBRACE};

    token->tokens.push_back(ip.create(t, it));
    return token;
}