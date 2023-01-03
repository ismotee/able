#pragma once
#include "token.h"

class PreProcessorError;

class PreProcessor
{
public:
    virtual bool check(Tokens &t, Tokens::iterator it) = 0;                                // check if this processor is the case for group of Tokens
    virtual std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it) = 0; // verify that statement is semantically sound
    virtual std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it) = 0;         // create a pre processed node that can be handled in parser
    size_t getLineLength() { return line.size(); }

    u_int endPosition;

protected:
    bool separateLine(Tokens &t, Tokens::iterator &begin, std::vector<TokenType> end = token::END); // separate line from the whole source
    Tokens line;                                                                                    // convenient cache for line specific statements
};

class DefinitionProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class AssignmentProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class ImportProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class ExpressionStatementProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class ExpressionProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class IdentifierProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);

    std::vector<TokenType> endsWith = token::END;
};

class ParameterProcessor : public PreProcessor
{
public:
    bool check(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens &t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens &t, Tokens::iterator &it);
};

class PreProcessorError
{
public:
    PreProcessorError(std::string message) : msg(message) {}

    std::string msg;
};

namespace preprocessor
{

    inline std::shared_ptr<PreProcessorError> error(std::string msg)
    {
        return std::make_shared<PreProcessorError>(msg);
    }

    inline bool expect(Tokens::iterator &it, std::vector<TokenType> expectedTypes)
    {
        it = std::next(it);
        return (*it)->isTypeOf(expectedTypes);
    }

    inline int expectMultiple(Tokens::iterator &it, std::vector<TokenType> expectedTokens)
    {
        int count = 0;
        while ((*it)->isTypeOf(expectedTokens))
        {
            if ((*it)->isTypeOf(token::END))
            {
                return -1;
            }
            ++count;
            it = std::next(it);
        }
        return count;
    }

    inline int fastForwardUntil(Tokens::iterator &it, std::vector<TokenType> expectedTokens)
    {
        int count = 0;
        while (!(*it)->isTypeOf(expectedTokens))
        {
            if ((*it)->isTypeOf(token::END))
            {
                return -1;
            }
            ++count;
            it = std::next(it);
        }
        return count;
    }
};