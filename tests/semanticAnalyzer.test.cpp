#include "gmock/gmock.h"
#include "lexer.h"
#include "semanticAnalyzer.h"
#include "errors.h"

using namespace ::testing;

class SemanticAnalyzerTest : public Test
{
public:
  Lexer l;
  SemanticAnalyzer a;

  void analyzeSource(std::string src)
  {
    l = Lexer(src);
    l.generateTokens();
    a = SemanticAnalyzer(l.tokens);
  }

  void compareSourceToVerbosedTokens(std::string source, std::string expectedVerbose)
  {
    try
    {
      analyzeSource(source);
      EXPECT_EQ(token::verboseTokens(a.analyzedTokens), expectedVerbose);
    }
    catch (SYNTAX_ERROR &e)
    {
      throw e;
    }
  }

  void expectSyntaxError(std::string source)
  {
    EXPECT_THROW({
      analyzeSource(source);
    },
                 SYNTAX_ERROR);
  }
};

TEST_F(SemanticAnalyzerTest, EmptyStaysEmpty)
{
  std::string source = "";
  analyzeSource(source);

  EXPECT_EQ(l.tokens, a.tokens);
}

TEST_F(SemanticAnalyzerTest, Links)
{
  std::string source = "[test][foo.md#ref]";
  std::string expectedVerbose = "IMPORT IDENTIFIER test FROM IDENTIFIER foo.md # ref";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

TEST_F(SemanticAnalyzerTest, Declarations)
{
  std::string source = "# foo\n"
                       "## bar (value x)";
  std::string expectedVerbose = "# IDENTIFIER foo\n"
                                "## IDENTIFIER bar PARAMETER value x";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

TEST_F(SemanticAnalyzerTest, Assignments)
{
  std::string source = "foo = bar";
  std::string expectedVerbose = "ASSIGN IDENTIFIER foo WITH EXPRESSION bar";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

// add cases
// list:
// - ...
// end of paragraph

TEST_F(SemanticAnalyzerTest, Calls)
{
  std::string source = "# foo bar\n"
                       "bar = 3\n"
                       "foo bar";
  std::string expectedVerbose = "# IDENTIFIER foo bar\n"
                                "ASSIGN IDENTIFIER bar WITH EXPRESSION 3\n"
                                "CALL foo bar";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

TEST_F(SemanticAnalyzerTest, CallWithArguments)
{
  std::string source = "# foo (x)\n"
                       "foo 3\n"
                       "# baz (y) bar\n"
                       "baz 4 bar";
  std::string expectedVerbose = "# IDENTIFIER foo PARAMETER x\n"
                                "CALL foo ARGUMENT 3\n"
                                "# IDENTIFIER baz PARAMETER y bar\n"
                                "CALL baz ARGUMENT 4 bar";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

TEST_F(SemanticAnalyzerTest, CallKeyPhrase)
{
  std::string source = "print 3\n"
                       "join list1 with list2";
  std::string expectedVerbose = "PRINT ARGUMENT EXPRESSION 3\n"
                                "JOIN ARGUMENT EXPRESSION list1 ARGUMENT EXPRESSION list2";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

TEST_F(SemanticAnalyzerTest, AssignmentWithOtherIdentifier)
{
  std::string source = "test = 3\n"
                       "other test = 3 + test";
  std::string expectedVerbose = "ASSIGN IDENTIFIER test WITH EXPRESSION 3\n"
                                "ASSIGN IDENTIFIER other test WITH EXPRESSION 3 + CALL test";
  compareSourceToVerbosedTokens(source, expectedVerbose);
}

// errors

TEST_F(SemanticAnalyzerTest, ImportErrorNotClosingBrackets)
{
  std::string source = "[test";
  expectSyntaxError(source);
}

TEST_F(SemanticAnalyzerTest, FromErrorNotClosingBrackets)
{
  std::string source = "[test][diip";
  expectSyntaxError(source);
}

TEST_F(SemanticAnalyzerTest, FromErrorUnexpectedCharacter)
{
  std::string source = "[test]foo";
  expectSyntaxError(source);
}

TEST_F(SemanticAnalyzerTest, DeclarationErrorNoName)
{
  std::string source = "#";
  expectSyntaxError(source);
}

/*
TEST_F(SemanticAnalyzerTest, CallErrorUnknownWord) {
  std::string source = "foo";
  expectSyntaxError(source);
}
*/
