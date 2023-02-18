#include <gmock/gmock.h>
#include "lexer.h"
#include "preparser.h"

using namespace ::testing;

class PreParserTest: public Test {
public:
  PreParserTest() {}

  void testPreParser(std::string src, std::string expected) {
    l = Lexer(src);
    l.generateTokens();

    pp = PreParser(l.tokens);
    pp.prepareFromStart();

    EXPECT_EQ(expected, token::verboseTokensWithoutSpaces(pp.parsedTokens));
  }

  void testIdentifiers(std::string src, std::string expected, bool verbose = false) {
    l = Lexer(src);
    l.generateTokens();

    pp = PreParser(l.tokens);
    pp.prepareFromStart();

    if (verbose)
      std::cout << pp.scoped->verboseIdentifiersRecursively();

    EXPECT_EQ(expected, pp.scoped->verboseIdentifiersRecursively());
  }

  Lexer l;
  PreParser pp;
};

TEST_F(PreParserTest, TestPreParser) {
  std::string src = "";
  std::string expected = "[0]\n";

  testPreParser(src, expected);
}

TEST_F(PreParserTest, TestPreParserComplete) {
  std::string src = "2 + 3 * test _1_ and _2_\n"
    "# test _foo_ and _bar_\n"
    "[description](#link-to-nearest-method)\n"
    "baz = 3";
  std::string expected = "[0]\n"
    "2 + 3 * test _1_ and _2_\n"
    "# test _foo_ and _bar_\n"
    "[1]\n"
    "[#link-to-nearest-method]\n"
    "baz = 3\n";

  testPreParser(src, expected);
}

TEST_F(PreParserTest, TestPreParserScopedIdentifiers) {
  std::string src = "test = 1\n"
    "# test _foo_ and _bar_\n"
    "## foobar\n"
    "bazzer = 2\n"
    "### baz\n"
    "deep = 3\n"
    "## yaid";

  std::string expected = "test\n"
    "test _foo_ and _bar_\n"
    "  foobar\n"
    "  yaid\n"
    "    bazzer\n"
    "    baz\n"
    "      deep\n";

  testIdentifiers(src, expected);
}