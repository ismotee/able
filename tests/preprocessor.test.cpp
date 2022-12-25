#include <gmock/gmock.h>
#include "preprocessor.h"
#include "lexer.h"

using namespace ::testing;

struct PreProcessorTest : public Test
{
    PreProcessorTest() {}

    void testDefinition(std::string src, std::string expected)
    {
        l = Lexer(src);
        l.generateTokens();

        Tokens t = l.tokens;
        Tokens::iterator it = t.begin();

        bool check = definition.check(t, it);
        auto error = definition.verify(t, it);
        auto result = definition.create(t, it);

        if (error != nullptr)
        {
            std::cout << error->msg << "\n";
        }

        EXPECT_TRUE(check);
        EXPECT_EQ(error, nullptr);
        EXPECT_EQ(result->verboseToken(), expected);
    }

    Lexer l;
    DefinitionProcessor definition;
};

TEST_F(PreProcessorTest, TestEmptyDefinition)
{
    std::string src = "# test";
    std::string expected = "# test (1) {}\n";

    testDefinition(src, expected);
}

TEST_F(PreProcessorTest, TestDefinitionDepth)
{
    std::string src = "#### tester";
    std::string expected = "# tester (4) {}\n";

    testDefinition(src, expected);
}