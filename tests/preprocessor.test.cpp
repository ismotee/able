#include <gmock/gmock.h>
#include "preprocessor.h"
#include "lexer.h"

using namespace ::testing;

struct PreProcessorTest: public Test {
    PreProcessorTest() {}

    Lexer l;
    DefinitionProcessor definition;
};

TEST_F(PreProcessorTest, TestEmptyDefinition) {
    std::string src = "# test";
    std::string expected = "";

    l = Lexer(src);
    l.generateTokens();

    Tokens t = l.tokens;
    Tokens::iterator it = t.begin();

    bool check = definition.check(t, it);
    auto error = definition.verify(t, it);
    auto result = definition.create(t, it);

    if (error != nullptr) {
        std::cout << error->msg << "\n";
    }

    EXPECT_TRUE(check);
    EXPECT_EQ(error, nullptr);
    EXPECT_EQ(result->verbose(), "IDENTIFIER BLOCK");
}