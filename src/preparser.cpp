#include "preparser.h"
#include "errors.h"
#include <algorithm>

void PreParser::prepareFromStart()
{
    parsedTokens.push_back(std::make_shared<PreBlock>());
    prepare(tokens.begin());
}

void PreParser::prepare(Tokens::iterator it)
{
    prepareStatements(it);
    flattenDeclarations();
    prepareScopeTree(parsedTokens.begin());
    gatherIdentifiers();
}

void PreParser::prepareStatements(Tokens::iterator &it)
{
    while (it != tokens.end() && !(*it)->isTypeOf(TokenType::END_OF_FILE))
    {
        for (auto processor : processors)
        {
            if (processor->check(tokens, it))
            {
                auto error = processor->verify(tokens, it);
                if (error == nullptr)
                {
                    parsedTokens.push_back(processor->create(tokens, it));
                    break;
                }
                else
                {
                    std::cerr << "Preprocessor error: " << error->msg << "\n";
                    std::exit(100);
                }
            }
        }
        it = std::next(it);
    }
}

void PreParser::flattenDeclarations()
{
    Tokens computed = {};
    auto it = parsedTokens.begin();
    while (it != parsedTokens.end())
    {
        if ((*it)->isTypeOf(TokenType::DECLARE))
        {
            auto def = std::dynamic_pointer_cast<PreDeclare>(*it);
            auto block = std::make_shared<PreBlock>();
            block->depth = def->depth;
            computed.push_back(def);
            computed.push_back(block);
        }
        else
        {
            computed.push_back(*it);
        }
        it = std::next(it);
    }
    parsedTokens = computed;
}

void PreParser::prepareScopeTree(Tokens::iterator it)
{
    ScopeBuilder builder = ScopeBuilder();
    builder.build(parsedTokens);
    scoped = builder.root;
}

void PreParser::gatherIdentifiers()
{
    gatherIdentifiersForScope(scoped);
}

void PreParser::gatherIdentifiersForScope(pPreScope scope)
{
    for (auto t : scope->tokens)
    {
        switch (t->type)
        {
        case TokenType::SCOPE:
        {
            gatherIdentifiersForScope(std::dynamic_pointer_cast<PreScope>(t));
            break;
        }
        case TokenType::DECLARE:
        {
            auto decl = std::dynamic_pointer_cast<PreDeclare>(t);

            while (decl->depth <= scope->depth)
            {
                scope = scope->broader;
            }

            scope->addIdentifier(decl->tokens[0]);
            break;
        }
        case TokenType::ASSIGNMENT:
        {
            auto assign = std::dynamic_pointer_cast<PreAssignment>(t);
            scope->addIdentifier(assign->tokens[0]);
            break;
        }
        default:
            break;
        }
    }
}