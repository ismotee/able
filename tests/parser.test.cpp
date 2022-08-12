#include <gmock/gmock.h>
#include "lexer.h"
#include "parser.h"

using namespace ::testing;

class ParserTest : public Test
{
public:
  ParserTest() {}

  void prepareSrc(std::string src, bool verbose)
  {
    Lexer l = Lexer(src);
    l.generateTokens();
    p = Parser(l.tokens);
    p.parse();
  }

  void compareSourceAndParserString(std::string src, std::string expected, bool verbose = false)
  {
    if (verbose)
    {
      std::cout << "testing:\n"
                << src << "\n\n";
      std::cout << "expected:\n"
                << expected << "\n";
    }

    prepareSrc(src, verbose);

    if (verbose)
    {
      std::cout << "result:\n"
                << p.toString();
    }

    EXPECT_EQ(expected, p.toString());
  }

  Parser p;
};


TEST_F(ParserTest, EmptySource) {
  std::string src = "";
  std::string expected = "{}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, EmptyDefinition) {
  std::string src = "# test";
  std::string expected = "{\n"
                         "  # test\n"
                         "  {}\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, NestedDefinitions) {
  std::string src = "# test\n"
                    "## too test\n"
                    "### deep test\n"
                    "## three test";
  std::string expected = "{\n"
                         "  # test\n"
                         "  {\n"
                         "    ## too test\n"
                         "    {\n"
                         "      ### deep test\n"
                         "      {}\n"
                         "    }\n"
                         "    ## three test\n"
                         "    {}\n"
                         "  }\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, DefinitionWithParameter) {
  std::string src = "# test (foo)\n"
                    "# (foo bar) test\n"
                    "# (foo) test (bar) test (baz) test (doodle doo)";
  std::string expected = "{\n"
                         "  # test (foo)\n"
                         "  {}\n"
                         "  # (foo bar) test\n"
                         "  {}\n"
                         "  # (foo) test (bar) test (baz) test (doodle doo)\n"
                         "  {}\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, SimpleExpressionsAndPrefixes) {
  std::string src = "5\n"
                    "-1\n"
                    "!3.5";
  std::string expected = "{\n"
                         "  5\n"
                         "  (-1)\n"
                         "  (!3.5)\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, InfixExpressions) {
  std::string src = "5 + 1\n"
                    "6 - 1\n"
                    "3 * 3\n"
                    "4 / 4";
  std::string expected = "{\n"
                         "  (5 + 1)\n"
                         "  (6 - 1)\n"
                         "  (3 * 3)\n"
                         "  (4 / 4)\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}

TEST_F(ParserTest, Precedence) {
  std::string src = "5 + 1 * -5 - 2 / 3";
  std::string expected = "{\n"
                         "  ((5 + (1 * (-5))) - (2 / 3))\n"
                         "}\n";

  compareSourceAndParserString(src, expected);
}


/*
TEST_F(ParserTest, prefixes)
{
  std::string src = "-2\n"
                    "4 + 5\n"
                    "2 + 3 * 5.2\n"
                    "-2 - 5\n"
                    "2 / -2\n"
                    "!5";
  std::string expectedStr = "(-2)\n"
                            "(4 + 5)\n"
                            "(2 + (3 * 5.2))\n"
                            "((-2) - 5)\n"
                            "(2 / (-2))\n"
                            "(!5)\n";
  compareSourceAndAstString(src, expectedStr);
}

TEST_F(ParserTest, definitionsAndCalls)
{
  std::string src = "test\n"
                    "# test";
  std::string expectedStr = "{test}\n"
                            "# test() {\n"
                            "}\n";
  compareSourceAndAstString(src, expectedStr);
}

TEST_F(ParserTest, definitionsAndCallsWithParams)
{
  std::string src = "test 4 + 3\n"
                    "# test (x)";
  std::string expectedStr = "{test(4 + 3)}\n"
                            "# test(x) {\n"
                            "}\n";
  compareSourceAndAstString(src, expectedStr);
}

TEST_F(ParserTest, WithMultipleParams)
{
  std::string src = "test 4 + 3 also -5\n"
                    "# test (x) also (foo y)";
  std::string expectedStr = "{test also(4 + 3)(-5)}\n"
                            "# test also(x, foo y) {\n"
                            "}\n";
  compareSourceAndAstString(src, expectedStr);
}

TEST_F(ParserTest, Assignment)
{
  std::string src = "test = 6\n"
                    "other test = 3 + test";
  std::string expectedStr = "test() = 6\n"
                            "other test() = (3 + {test})\n";

  compareSourceAndAstString(src, expectedStr);
}

TEST_F(ParserTest, AssignmentWithParams)
{
  std::string src = "test (x) = x + 1";
  std::string expectedStr = "test(x) = ({x} + 1)\n";

  compareSourceAndAstString(src, expectedStr, true);
}
*/