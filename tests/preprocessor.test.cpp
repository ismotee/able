#include <gmock/gmock.h>
#include "preprocessor.h"
#include "lexer.h"

using namespace ::testing;

struct PreProcessorTest : public Test
{
    PreProcessorTest() {}

    void testProcessor(std::shared_ptr<PreProcessor> p, std::string src, std::string expected)
    {
        l = Lexer(src);
        l.generateTokens();
        Tokens t = l.tokens;
        Tokens::iterator it = t.begin();

        bool check = p->check(t, it);
        auto error = p->verify(t, it);
        auto result = p->create(t, it);

        if (error != nullptr)
        {
            std::cout << error->msg << "\n";
        }

        EXPECT_TRUE(check);
        EXPECT_EQ(error, nullptr);
        EXPECT_EQ(result->verboseToken(), expected);
    }

    void testDefinition(std::string src, std::string expected)
    {
        testProcessor(std::make_shared<DefinitionProcessor>(), src, expected);
    }

    void testAssignment(std::string src, std::string expected)
    {
        testProcessor(std::make_shared<AssignmentProcessor>(), src, expected);
    }

    void testImport(std::string src, std::string expected)
    {
        testProcessor(std::make_shared<ImportProcessor>(), src, expected);
    }

    void testExpressionStatement(std::string src, std::string expected)
    {
        testProcessor(std::make_shared<ExpressionStatementProcessor>(), src, expected);
    }

    Lexer l;
};

TEST_F(PreProcessorTest, TestEmptyDefinition)
{
    std::string src = "# test";
    std::string expected = "# test\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestDefinitionDepth)
{
    std::string src = "#### tester";
    std::string expected = "# tester\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestDefinitionWithMultipleWords)
{
    std::string src = "# test foo bar";
    std::string expected = "# test foo bar\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestDefinitionWithParameters)
{
    std::string src = "# test (foo) bar";
    std::string expected = "# test (foo) bar\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestDefinitionWithMultipleParameters)
{
    std::string src = "# (other param) test (foo) bar";
    std::string expected = "# (other param) test (foo) bar\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestAssignment)
{
    std::string src = "test = 1";
    std::string expected = "test = 1\n";

    testAssignment(src, expected);
}

TEST_F(PreProcessorTest, TestAssignmentWithComplexExpression)
{
    std::string src = "test = (1 + 5) * foo bar";
    std::string expected = "test = ( 1 + 5 ) * foo bar\n";

    testAssignment(src, expected);
}

TEST_F(PreProcessorTest, TestAssignmentWithImport)
{
    std::string src = "test = [foo](#bar)";
    std::string expected = "test = [#bar]\n";

    testAssignment(src, expected);
}

TEST_F(PreProcessorTest, TestExpressionStatement)
{
    std::string src = "(1 + 5) * foo bar";
    std::string expected = "( 1 + 5 ) * foo bar\n";

    testExpressionStatement(src, expected);
}

TEST_F(PreProcessorTest, TestEmptyExpressionStatement)
{
    std::string src = "";
    std::string expected = "\n";

    testExpressionStatement(src, expected);
}

TEST_F(PreProcessorTest, TestImport)
{
    std::string src = "[foo bar 2 + 3](#test)";
    std::string expected = "[#test]";

    testImport(src, expected);
}