#pragma once
#include "token.h"

class PreProcessorError;

class PreProcessor {
public:
    virtual bool check(Tokens& t, Tokens::iterator it) = 0;                                // check if this processor is the case for group of Tokens
    virtual std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it) = 0; // verify that statement is semantically sound
    virtual std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it) = 0;         // create a pre processed node that can be handled in parser
    size_t getLineLength() { return line.size(); }

    u_int endPosition;

protected:
    bool separateLine(Tokens& t, Tokens::iterator& begin, std::vector<TokenType> end = token::END); // separate line from the whole source
    Tokens line;                                                                                    // convenient cache for line specific statements
};

class ComponentProcessor: public PreProcessor {
public:
    TokenTypes endsWith = token::END;
};

class DefinitionProcessor: public PreProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class AssignmentProcessor: public PreProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class ImportProcessor: public PreProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class ExpressionStatementProcessor: public PreProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class ExpressionProcessor: public ComponentProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);

    TokenTypes legalTokens = { TokenType::EQUALS_COMPARE, TokenType::WORD,
                         TokenType::PLUS, TokenType::MINUS,
                         TokenType::SLASH, TokenType::ASTERISK,
                         TokenType::COLON, TokenType::BANG,
                         TokenType::LBRACE, TokenType::RBRACE,
                         TokenType::LBRACKET, TokenType::RBRACKET,
                         TokenType::GT, TokenType::GT_OR_EQUALS,
                         TokenType::LT, TokenType::LT_OR_EQUALS,
                         TokenType::NOT_EQUALS, TokenType::NUMBER,
                         TokenType::WORD, TokenType::UNDERSCORE, };
};

class ParameterProcessor: public PreProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
protected:
    std::shared_ptr<PreProcessorError> verifyParameter(Tokens& t, Tokens::iterator& it);
    std::shared_ptr<PreToken> createParameter(Tokens& t, Tokens::iterator& it, std::shared_ptr<ComponentProcessor> cp);
};

class IdentifierProcessor: public ComponentProcessor {
public:
    bool check(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
    void setEndsWith(TokenTypes tt) {
        endsWith = tt;
    }

    TokenTypes endsWith = {
        TokenType::PLUS, TokenType::MINUS,
        TokenType::SLASH, TokenType::ASTERISK,
        TokenType::END_OF_FILE, TokenType::ENDL,
        TokenType::BANG,
        TokenType::GT, TokenType::GT_OR_EQUALS,
        TokenType::LT, TokenType::LT_OR_EQUALS,
        TokenType::EQUALS_COMPARE, TokenType::NOT_EQUALS,
    };

protected:
    virtual std::shared_ptr<PreProcessorError> verifyIdentifier(Tokens& t, Tokens::iterator it, std::shared_ptr<ParameterProcessor> pp);
    virtual std::shared_ptr<PreToken> createIdentifier(Tokens& t, Tokens::iterator& it, std::shared_ptr<ParameterProcessor> pp);
};

class DeclarationIdentifierProcessor: public IdentifierProcessor {
public:
    DeclarationIdentifierProcessor(): IdentifierProcessor() {
        setEndsWith(token::END);
    }
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it) override;
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it) override;
};


class ActualParameterProcessor: public ParameterProcessor {
public:
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class FormalParameterProcessor: public ParameterProcessor {
public:
    std::shared_ptr<PreProcessorError> verify(Tokens& t, Tokens::iterator it);
    std::shared_ptr<PreToken> create(Tokens& t, Tokens::iterator& it);
};

class PreProcessorError {
public:
    PreProcessorError(std::string message): msg(message) {}

    std::string msg;
};

namespace preprocessor {

    inline std::shared_ptr<PreProcessorError> error(std::string msg) {
        return std::make_shared<PreProcessorError>(msg);
    }

    inline bool expect(Tokens::iterator& it, std::vector<TokenType> expectedTypes) {
        it = std::next(it);
        return (*it)->isTypeOf(expectedTypes);
    }

    inline int expectMultiple(Tokens::iterator& it, std::vector<TokenType> expectedTokens) {
        int count = 0;
        while ((*it)->isTypeOf(expectedTokens)) {
            if ((*it)->isTypeOf(token::END)) {
                return -1;
            }
            ++count;
            it = std::next(it);
        }
        return count;
    }

    inline int fastForwardUntil(Tokens::iterator& it, std::vector<TokenType> expectedTokens) {
        int count = 0;
        while (!(*it)->isTypeOf(expectedTokens)) {
            if ((*it)->isTypeOf(token::END)) {
                return -1;
            }
            ++count;
            it = std::next(it);
        }
        return count;
    }
};