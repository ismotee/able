#pragma once
#include <unordered_map>
#include "token.h"
#include <iostream>
/*
namespace keyphrases {
    // print ARGUMENT
    static std::shared_ptr<Token> kPrint1 = std::make_shared<Word>("print");
    //    static std::shared_ptr<Token> kPrint2 = std::make_shared<Parameter>();

    static std::shared_ptr<Token> cPrint1 = std::make_shared<Print>();
    //    static std::shared_ptr<Token> cPrint2 = std::make_shared<Argument>();

    static Tokens keyPrint = { kPrint1, kPrint2 };
    static Tokens callPrint = { cPrint1, cPrint2 };

    static std::shared_ptr<Token> kJoin1 = std::make_shared<Word>("join");
    //     static std::shared_ptr<Token> kJoin2 = std::make_shared<Parameter>();
    static std::shared_ptr<Token> kJoin3 = std::make_shared<Word>("with");
    //    static std::shared_ptr<Token> kJoin4 = std::make_shared<Parameter>();

    static std::shared_ptr<Token> cJoin1 = std::make_shared<Join>();
    static std::shared_ptr<Token> cJoin2 = std::make_shared<Argument>();
    static std::shared_ptr<Token> cJoin3 = std::make_shared<Argument>();

    static Tokens keyJoin = { kJoin1, kJoin2, kJoin3, kJoin4 };
    static Tokens callJoin = { cJoin1, cJoin2, cJoin3 };

    static std::unordered_map<std::string, Tokens> builtinKeyPhrases = {
        {"PRINT", keyPrint},
        {"JOIN", keyJoin} };

    static std::unordered_map<std::string, Tokens> builtinPhraseCalls = {
        {"PRINT", callPrint},
        {"JOIN", callJoin} };

    static void prepare() {
        for (auto& phrase : builtinKeyPhrases) {
            token::linkTokens(phrase.second);
        }

        for (auto& phrase : builtinPhraseCalls) {
            token::linkTokens(phrase.second);
        }
    }
}

*/