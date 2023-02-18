#include <gmock/gmock.h>
#include "lexer.h"
#include "parser.h"
#include "preparser.h"

using namespace ::testing;

class ParserTest: public Test {
public:
  ParserTest() {}
  void testParser(std::string src, std::string expected, std::string astTree) {
    l = Lexer(src);
    l.generateTokens();
    pp = PreParser(l.tokens);
    pp.prepareFromStart();
    p = Parser(pp.scoped);
    p.parseScope();

    EXPECT_EQ(expected, p.scope->toString());
    EXPECT_EQ(astTree, p.scope->typeToString(0));
  }
  Lexer l;
  PreParser pp;
  Parser p;

};

TEST_F(ParserTest, ParseExpressionStatement) {
  std::string src = "!-1 + 3 == 4 * 3";
  std::string expected = "((!-1 + 3) == (4 * 3))\n";

  std::string astTree = "PROGRAM\n"
    "  EXPRESSION_STATEMENT->!-NUMBER + NUMBER == NUMBER * NUMBER\n";

  testParser(src, expected, astTree);
}

TEST_F(ParserTest, ParseCall) {
  std::string src = "test\n"
    "# test";
  std::string expected = "test\n"
    "# test\n";

  std::string astTree = "PROGRAM\n"
    "  EXPRESSION_STATEMENT->CALL(0)\n"
    "  DECLARATION IDENTIFIER\n"
    "  PROGRAM\n";

  testParser(src, expected, astTree);
}


TEST_F(ParserTest, ParseCallWithArgument) {
  std::string src = "test _1_\n"
    "# test _foo_";
  std::string expected = "test _1_\n"
    "# test _foo_\n";

  std::string astTree = "PROGRAM\n"
    "  EXPRESSION_STATEMENT->CALL(1)\n"
    "  DECLARATION IDENTIFIER\n"
    "  PROGRAM\n";

  testParser(src, expected, astTree);
}
