#include "preparser.h"
#include "errors.h"
#include <algorithm>

void PreParser::prepareFromStart() {
    parsedTokens.push_back(std::shared_ptr<PreBlock>());
    prepare(tokens.begin());
}

void PreParser::prepare(Tokens::iterator it) {
    while (it != tokens.end()) {
        for (auto processor : processors) {
            if (processor->check(tokens, it)) {
                auto error = processor->verify(tokens, it);
                if (error == nullptr) {
                    parsedTokens.push_back(processor->create(tokens, it));
                    break;
                } else {
                    std::cerr << "Preprocessor error: " << error->msg << "\n";
                    std::exit(100);
                }
            }
        }
        it = std::next(it);
    }
}

