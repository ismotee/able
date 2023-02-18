#include "gmock/gmock.h"
#include "lexer.h"
#include "tokenType.h"
#include "errors.h"

using namespace ::testing;

struct TokenTester {
  TokenTester() {}
  TokenTester(TokenType t, std::string l): type(t), literal(l) {}
  TokenType type;
  std::string literal;
};

typedef std::vector<TokenTester> TokenTesters;

class LexerTest: public Test {
public:
  Lexer l;
  void generateTokensFromSource(std::string src) {
    l = Lexer(src);
    l.generateTokens();
  }
  void testTokens(TokenTesters expectedTokens) {
    EXPECT_EQ(l.tokens.size(), expectedTokens.size());
    for (u_int i = 0; i < l.tokens.size(); ++i) {
      TokenTester tester = expectedTokens[i];
      auto token = l.tokens[i];
      // test type
      TokenType expectedType = tester.type;
      TokenType tokenType = token->type;
      EXPECT_EQ(tokenTypeToString[tokenType], tokenTypeToString[expectedType]);

      // test literal
      std::string expectedLiteral = tester.literal;
      std::string literal = token->literal;
      EXPECT_EQ(literal, expectedLiteral);
    }
  }
};

TEST_F(LexerTest, AlwaysEOF) {
  std::string source = "";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, SingleWordDeclaration) {
  std::string source = "# moi";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::HASH, "#"},
      {TokenType::WORD, "moi"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, MultipleWordDeclaration) {
  std::string source = "# moi hei";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::HASH, "#"},
      {TokenType::WORD, "moi"},
      {TokenType::WORD, "hei"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, MultipleDeclarationMarks) {
  std::string source = "## moi";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::HASH, "#"},
      {TokenType::HASH, "#"},
      {TokenType::WORD, "moi"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, ArgumentsInDeclaration) {
  std::string source = "# from _foo_ to _bar and baz_";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::HASH, "#"},
      {TokenType::WORD, "from"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::WORD, "foo"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::WORD, "to"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::WORD, "bar"},
      {TokenType::WORD, "and"},
      {TokenType::WORD, "baz"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, Calls) {
  std::string source = "from _1_ to _3 + foo_";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::WORD, "from"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::NUMBER, "1"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::WORD, "to"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::NUMBER, "3"},
      {TokenType::PLUS, "+"},
      {TokenType::WORD, "foo"},
      {TokenType::UNDERSCORE, "_"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, MultipleLines) {
  std::string source = "# foo\n"
    "bar";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::HASH, "#"},
      {TokenType::WORD, "foo"},
      {TokenType::ENDL, "\n"},
      {TokenType::WORD, "bar"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, ColonAssignment) {
  std::string source = "foo: bar";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::WORD, "foo"},
      {TokenType::COLON, ":"},
      {TokenType::WORD, "bar"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, Numbers) {
  std::string source = "10 1.0 .234";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::NUMBER, "10"},
      {TokenType::NUMBER, "1.0"},
      {TokenType::NUMBER, ".234"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, LinkedMethods) {
  std::string source = "[2 times 5](times.md#x-times-y)";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::LBRACKET, "["},
      {TokenType::NUMBER, "2"},
      {TokenType::WORD, "times"},
      {TokenType::NUMBER, "5"},
      {TokenType::RBRACKET, "]"},
      {TokenType::LBRACE, "("},
      {TokenType::WORD, "times.md"},
      {TokenType::HASH, "#"},
      {TokenType::WORD, "x-times-y"},
      {TokenType::RBRACE, ")"},
      {TokenType::END_OF_FILE, ""} };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, ArithmeticOperators) {
  std::string source = "- + * / =";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::MINUS, "-"},
      {TokenType::PLUS, "+"},
      {TokenType::ASTERISK, "*"},
      {TokenType::SLASH, "/"},
      {TokenType::EQUALS, "="},
      {TokenType::END_OF_FILE, ""},
  };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, Modifiers) {
  std::string source = "!";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::BANG, "!"},
      {TokenType::END_OF_FILE, ""},
  };

  testTokens(expectedTokens);
}

TEST_F(LexerTest, LogicalOperators) {
  std::string source = "== != > < >= <=";
  generateTokensFromSource(source);

  TokenTesters expectedTokens = {
      {TokenType::EQUALS_COMPARE, "=="},
      {TokenType::NOT_EQUALS, "!="},
      {TokenType::GT, ">"},
      {TokenType::LT, "<"},
      {TokenType::GT_OR_EQUALS, ">="},
      {TokenType::LT_OR_EQUALS, "<="},
      {TokenType::END_OF_FILE, ""},
  };

  testTokens(expectedTokens);
}

// syntax errors
TEST_F(LexerTest, TooManyDots) {
  EXPECT_THROW({
    std::string source = "1.0.0";
    generateTokensFromSource(source);
    },
    SYNTAX_ERROR);
}