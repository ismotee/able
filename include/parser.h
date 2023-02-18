#pragma once
#include <memory>
#include <iterator>
#include "ast.h" 
#include "token.h"
#include "tokenType.h" 

enum class ExprOrder {
  LOWEST = 0,
  EQUALS,
  LESSGREATER,
  SUM,
  PPRODUCT,
  PREFIX
};

static std::unordered_map<TokenType, ExprOrder> precedences = {
    {TokenType::ENDL, ExprOrder::LOWEST},
    {TokenType::END_OF_FILE, ExprOrder::LOWEST},
    {TokenType::EQUALS_COMPARE, ExprOrder::EQUALS},
    {TokenType::NOT_EQUALS, ExprOrder::EQUALS},
    {TokenType::LT, ExprOrder::LESSGREATER},
    {TokenType::GT, ExprOrder::LESSGREATER},
    {TokenType::LT_OR_EQUALS, ExprOrder::LESSGREATER},
    {TokenType::GT_OR_EQUALS, ExprOrder::LESSGREATER},
    {TokenType::PLUS, ExprOrder::SUM},
    {TokenType::MINUS, ExprOrder::SUM},
    {TokenType::SLASH, ExprOrder::PPRODUCT},
    {TokenType::ASTERISK, ExprOrder::PPRODUCT},
};

struct Block;
struct Scope;
struct Phrase;
struct Param;

typedef std::shared_ptr<Block> pBlock;
typedef std::shared_ptr<Scope> pScope;
typedef std::vector<pScope> Scopes;
typedef std::shared_ptr<Phrase> pPhrase;
typedef std::shared_ptr<Param> pParam;
typedef std::vector<pParam> Params;

static std::string trimTrailingWhiteSpace(std::string str) {
  while (str.size() > 0 && str[str.size() - 1] == ' ') {
    str.pop_back();
  }
  return str;
}

struct Param: public AstNode {
  Param(Tokens t = {}): tokens(t) {}

  Tokens tokens;

  static pParam New(Tokens tokens = {}) {
    return std::make_shared<Param>(tokens);
  }

  std::string toString() {
    std::string res = "(";

    for (auto token : tokens) {
      res += token->literal;
      res += " ";
    }

    res = trimTrailingWhiteSpace(res);
    res += ")";

    return res;
  }
};

struct Phrase {
  Phrase(Tokens t = {}): tokens(t), params({}) {}

  Tokens tokens;
  Params params;

  inline bool operator==(const Tokens& t) {
    auto cur = tokens.begin();
    for (auto it = t.begin(); it != t.end();) {
      // if phrase ends before comparison tokens
      if (cur == tokens.end()) return false;

      // if tokens match
      if (*cur == *it) {
        ++it;
        ++cur;
        continue;
      }

      // if there's a parameter
      if ((*cur)->isTypeOf(TokenType::PARAMETER)) {
        ++cur;

        // if parameter is at the end of the phrase
        if (cur == tokens.end()) return true;

        // fast-forward parameter to a next matching token
        for (;(*it) != (*cur);) {
          ++it;

          // if comparison tokens end before finding a matching token
          if (it == t.end()) {
            return false;
          }
        }
      }
    }
    return true;
  }

  std::string toString() {
    std::string res;
    u_int i_param = 0;

    for (auto token : tokens) {
      if (token->isTypeOf(TokenType::PARAMETER)) {
        res += params[i_param]->toString();
        ++i_param;
      } else {
        res += token->literal;
      }
      res += " ";
    }

    res = trimTrailingWhiteSpace(res);
    res += "\n";
    return res;
  }

  static pPhrase New(Tokens tokens = {}) {
    return std::make_shared<Phrase>(tokens);
  }
};


struct Block: public AstNode {
  Block(SemanticTokens t = {}): tokens(t) {}

  u_int depth;
  SemanticTokens tokens;


  std::string toString() {
    std::string res;

    res += intend();
    res += "{";
    if (tokens.size() > 0) res += "\n";

    for (pAstNode token : tokens) {
      res += intend(true);
      res += token->toString();
    }

    if (tokens.size() > 0) res += intend();
    res += "}\n";

    return res;
  }

  void setDepth(u_int d) {
    depth = d;
  }

  std::string intend(bool isInside = false) {
    std::string res;
    u_int limit = depth;

    if (isInside) {
      ++limit;
    }

    for (u_int i = 0; i < limit; ++i) {
      res += "  ";
    }

    return res;
  }

  static pBlock New(SemanticTokens tokens = {}) {
    return std::make_shared<Block>(tokens);
  }
};

struct Scope: public AstNode {
  Scope(pScope _broader = nullptr, pPhrase i = nullptr):
    block(Block::New()),
    identifier(i) {
    setBroaderScope(_broader);
  }

  pPhrase identifier;
  pBlock block;
  pScope broader;
  Scopes narrower;

  u_int getDepth() {
    return block->depth;
  }

  void addLine(pAstNode line) {
    block->tokens.push_back(line);
  }

  void setBroaderScope(pScope _broader) {
    broader = _broader;
    if (broader != nullptr) {
      block->setDepth(broader->getDepth() + 1);
    } else {
      block->setDepth(0);
    }
  }

  std::string toString() {
    std::string res;
    if (identifier != nullptr) {
      for (u_int i = 0; i < getDepth(); ++i) res += "#";
      res += " " + identifier->toString();
    }

    res += block->toString();
    return res;
  }

  static pScope New(pScope broader = nullptr, pPhrase identifier = nullptr) {
    return std::make_shared<Scope>(broader, identifier);
  }
};



class Parser {
public:
  Parser(pPreScope p = nullptr): pre(p) {}
  void parseScope();
  void parseStatement(pToken p);
  pParserError verifyStatement(pToken t);
  pAstStatement createStatement(pToken t);

  pPreScope pre;
  pAstProgram scope = std::make_shared<AstProgram>();
  Processors processors = {
      std::make_shared<ExpressionStatementProc>(pre),
      std::make_shared<DeclarationProc>(pre) };

protected:
  pParserError newPError(std::string msg) {
    return std::make_shared<ParserError>(msg);
  }

  Tokens::iterator cur;
  Tokens::iterator prev;
};
