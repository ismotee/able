#include <gmock/gmock.h>
#include "lexer.h"
#include "preparser.h"
#include "parser.h"

using namespace ::testing;

TEST(ParseExpressionStatement)
{
  std::string src = "1";
  std::string verbosed = "[0]\n"
                         "1\n";

  std::string astTree = "SCOPE\n"
                        "EXPRESSION_STATEMENT\n";

  Lexer l = Lexer(src);
  l.generateTokens();
  PreParser pp = PreParser(l.tokens);
  pp.prepareFromStart();
  Parser p = Parser();
  p.parseScope(pp.scoped);

  EXPECT_EQ(src, p.scope->toString());
}