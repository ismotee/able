#pragma once
#include <unordered_map>

enum class TokenType {
    UNDEFINED,

    MINUS,
    PLUS,
    ASTERISK,
    SLASH,
    EQUALS,

    WORD,
    NUMBER,

    HASH,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COLON,
    BANG,
    UNDERSCORE,

    EQUALS_COMPARE,
    NOT_EQUALS,
    GT,
    LT,
    GT_OR_EQUALS,
    LT_OR_EQUALS,

    PRE_TOKEN,
    IMPORT,
    FROM,
    DECLARE,
    ASSIGNMENT,
    EXPRESSION_STATEMENT,
    WITH,
    IDENTIFIER,
    PARAMETER,
    EXPRESSION,
    CALL,
    ARGUMENT,
    BLOCK,
    SCOPE,

    PRINT,
    JOIN,

    ENDL,
    END_OF_FILE,
};

typedef std::vector<TokenType> TokenTypes;

static std::unordered_map<std::string, TokenType> stringToTokenType{
    {"undefined", TokenType::UNDEFINED},

    { "-", TokenType::MINUS },
    { "+", TokenType::PLUS },
    { "*", TokenType::ASTERISK },
    { "/", TokenType::SLASH },
    { "=", TokenType::EQUALS },

    { "word", TokenType::WORD },
    { "number", TokenType::NUMBER },

    { "#", TokenType::HASH },
    { "(", TokenType::LBRACE },
    { ")", TokenType::RBRACE },
    { "[", TokenType::LBRACKET },
    { "]", TokenType::RBRACKET },
    { ":", TokenType::COLON },
    { "!", TokenType::BANG },
    { "_", TokenType::UNDERSCORE },

    { "==", TokenType::EQUALS_COMPARE },
    { "!=", TokenType::NOT_EQUALS },
    { ">", TokenType::GT },
    { "<", TokenType::LT },
    { ">=", TokenType::GT_OR_EQUALS },
    { "<=", TokenType::LT_OR_EQUALS },

    { "IMPORT", TokenType::IMPORT },
    { "FROM", TokenType::FROM },
    { "DECLARE", TokenType::DECLARE },
    { "ASSIGN", TokenType::ASSIGN },
    { "WITH", TokenType::WITH },
    { "IDENTIFIER", TokenType::IDENTIFIER },
    { "PARAMETER", TokenType::PARAMETER },
    { "EXPRESSION", TokenType::EXPRESSION },
    { "EXPRESSION_STATEMENT", TokenType::EXPRESSION_STATEMENT },
    { "CALL", TokenType::CALL },
    { "ARGUMENT", TokenType::ARGUMENT },

    { "PRINT", TokenType::PRINT },
    { "JOIN", TokenType::JOIN },

    { "\n", TokenType::ENDL },
    { "", TokenType::END_OF_FILE },
};

static std::unordered_map<TokenType, std::string> tokenTypeToString{
    {TokenType::UNDEFINED, "undefined"},

    { TokenType::MINUS, "-" },
    { TokenType::PLUS, "+" },
    { TokenType::ASTERISK, "*" },
    { TokenType::SLASH, "/" },
    { TokenType::EQUALS, "=" },

    { TokenType::WORD, "word" },
    { TokenType::NUMBER, "number" },

    { TokenType::HASH, "#" },
    { TokenType::LBRACE, "(" },
    { TokenType::RBRACE, ")" },
    { TokenType::LBRACKET, "[" },
    { TokenType::RBRACKET, "]" },
    { TokenType::COLON, ":" },
    { TokenType::BANG, "!" },
    { TokenType::UNDERSCORE, "_" },

    { TokenType::EQUALS_COMPARE, "==" },
    { TokenType::NOT_EQUALS, "!=" },
    { TokenType::GT, ">" },
    { TokenType::LT, "<" },
    { TokenType::GT_OR_EQUALS, ">=" },
    { TokenType::LT_OR_EQUALS, "<=" },

    { TokenType::IMPORT, "IMPORT" },
    { TokenType::FROM, "FROM" },
    { TokenType::DECLARE, "DECLARE" },
    { TokenType::ASSIGN, "ASSIGN" },
    { TokenType::WITH, "WITH" },
    { TokenType::IDENTIFIER, "IDENTIFIER" },
    { TokenType::PARAMETER, "PARAMETER" },
    { TokenType::EXPRESSION, "EXPRESSION" },
    { TokenType::EXPRESSION_STATEMENT, "EXPRESSION_STATEMENT" },
    { TokenType::CALL, "CALL" },
    { TokenType::ARGUMENT, "ARGUMENT" },

    { TokenType::PRINT, "PRINT" },
    { TokenType::JOIN, "JOIN" },

    { TokenType::ENDL, "\n" },
    { TokenType::END_OF_FILE, "EOF" } };
