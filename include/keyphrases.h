#pragma once
#include <unordered_map>
#include "token.h"



namespace keyphrases {
    // print ARGUMENT
    static std::shared_ptr<Token> kp1 = std::make_shared<Word>("print");
    static std::shared_ptr<Token> kp2 = std::make_shared<Parameter>(kp1);
    static std::shared_ptr<Token> kp3 = std::make_shared<Word>("out", kp2);

    static std::shared_ptr<Token> cp1 = std::make_shared<Print>();
    static std::shared_ptr<Token> cp2 = std::make_shared<Argument>();

    static Tokens keyPrint = { kp1, kp2, kp3 };
    static Tokens callPrint = { cp1, cp2 };

    static std::unordered_map<std::string, Tokens> builtinKeyPhrases = {
        {"PRINT", keyPrint} };

    static std::unordered_map<std::string, Tokens> builtinPhraseCalls = {
        {"PRINT", callPrint} };

    static void prepare() {
        for (auto& phrase : builtinKeyPhrases) {
            token::linkTokens(phrase.second);
        }

        for (auto& phrase : builtinPhraseCalls) {
            token::linkTokens(phrase.second);
        }
    }
}
