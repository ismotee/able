#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "token.h"

struct AstNode;
typedef std::shared_ptr<AstNode> pAstNode;
typedef std::vector<pAstNode> SemanticTokens;

struct AstProgram;
typedef std::shared_ptr<AstProgram> pAstProgram;

struct AstNode {
  AstNode(std::shared_ptr<Token> t = nullptr): token(t) {}
  virtual std::string tokenLiteral() {
    return token->literal;
  }
  virtual std::string toString() = 0;
  virtual std::string typeToString(u_int in) = 0;

  std::string intend(u_int in) {
    std::string res = "";
    for (u_int i = 0; i < in; ++i) {
      res += "  ";
    }
    return res;
  }

  std::shared_ptr<Token> token;
};

struct AstStatement: AstNode {
  AstStatement(std::shared_ptr<Token> t = nullptr): AstNode(t) {}
};

struct AstExpression: AstNode {
  AstExpression(std::shared_ptr<Token> t = nullptr): AstNode(t) {}
};

typedef std::vector<std::shared_ptr<AstStatement>> Statements;

struct AstProgram: AstNode {
  Statements statements;

  void addStatement(pAstStatement s) {
    statements.push_back(s);
  }

  std::string tokenLiteral() override {
    if (statements.size() > 0) {
      return statements[0]->tokenLiteral();
    } else {
      return "";
    }
  }

  std::string toString() {
    std::string out;
    for (auto stmt : statements) {
      out += stmt->toString();
    }
    return out;
  }

  std::string typeToString(u_int in) {
    std::string out = intend(in) + "PROGRAM\n";
    for (auto stmt : statements) {
      out += stmt->typeToString(in + 1);
    }
    return out;
  }

};

struct AstNumber: AstExpression {
  AstNumber() {}
  AstNumber(pToken t, double v): AstExpression(t), value(v) {}

  double value;

  std::string toString() {
    std::string nbr = std::to_string(value);
    if (nbr.size() != 1) {
      for (auto it = nbr.end() - 1; (*it) == '0' || (*it) == '.'; --it) {
        nbr.erase(it);
      }
    }
    return nbr;
  }

  std::string typeToString(u_int in) {
    return "NUMBER";
  }

};

struct AstExpressionStatement: AstStatement {
  AstExpressionStatement() {}
  AstExpressionStatement(std::shared_ptr<Token> t, std::shared_ptr<AstExpression> e): AstStatement(t), expression(e) {}

  std::shared_ptr<AstExpression> expression;

  std::string toString() {
    if (expression != nullptr) {
      return expression->toString() + "\n";
    }
    return "";
  }

  std::string typeToString(u_int in) {
    return intend(in) + "EXPRESSION_STATEMENT->" + expression->typeToString(in) + "\n";
  }
};

struct AstPrefixExpression: AstExpression {
  AstPrefixExpression(): AstExpression() {}
  AstPrefixExpression(std::shared_ptr<Token> t, std::shared_ptr<AstExpression> e): AstExpression(t), right(e) {}

  std::shared_ptr<AstExpression> right;

  std::string toString() {
    return token->literal + right->toString();
  }

  std::string typeToString(u_int in) {
    return tokenTypeToString[token->type] + right->typeToString(in);
  }

};

struct AstCall: public AstExpression {
  AstCall(): AstExpression(), identifier(nullptr) {}
  AstCall(pToken t, pPreIdentifier id): AstExpression(t), identifier(id) {}

  pPreIdentifier identifier;

  std::string toString() {
    return identifier->verboseToken();
  }

  std::string typeToString(u_int in) {
    u_int numParams = 0;
    for (auto t : identifier->tokens) {
      if (t->isTypeOf(TokenType::PARAMETER)) {
        ++numParams;
      }
    }
    return "CALL(" + std::to_string(numParams) + ")";
  }
};

typedef std::shared_ptr<AstCall> pAstCall;

struct AstInfixExpression: AstExpression {
  AstInfixExpression() {}
  AstInfixExpression(
    std::shared_ptr<Token> t,
    std::shared_ptr<AstExpression> l = nullptr,
    std::shared_ptr<AstExpression> r = nullptr): AstExpression(t),
    left(l),
    right(r) {}

  std::shared_ptr<AstExpression> left;
  std::shared_ptr<AstExpression> right;

  std::string toString() {
    return "(" + left->toString() + " " + token->literal + " " + right->toString() + ")";
  }

  std::string typeToString(u_int in) {
    return left->typeToString(in) +
      " " + tokenTypeToString[token->type] +
      " " + right->typeToString(in);
  }

};

struct AstIdentifier: public AstExpression {
  AstIdentifier(pToken t, pPreIdentifier _name): AstExpression(t), name(_name) {}

  // this probably is not necessary since the same token is stored to AstNode::token
  pPreIdentifier name;

  std::string toString() {
    return name->verboseToken();
  }

  std::string typeToString(u_int in) {
    return "IDENTIFIER";
  }
};

typedef std::shared_ptr<AstIdentifier> pAstIdentifier;

struct AstParameter: public AstExpression {
  AstParameter() {}
  AstParameter(std::shared_ptr<Token> t): AstExpression(t) {}

  std::string toString() { return ""; }
  std::string typeToString(u_int in) { return "PARAM"; }
};

typedef std::vector<std::shared_ptr<AstParameter>> Parameters;

struct AstDeclarationStatement: public AstStatement {
  AstDeclarationStatement() {}
  AstDeclarationStatement(std::shared_ptr<Token> t, pAstIdentifier id): AstStatement(t), name(id) {}

  pAstIdentifier name;
  pAstProgram scope;

  std::string toString() {
    std::string res = "# ";
    res += name->toString() + "\n";
    res += scope->toString();
    return res;
  }

  std::string typeToString(u_int in) {
    return intend(in) + "DECLARATION " + name->typeToString(in) + "\n"
      + scope->typeToString(in);
  }
};

struct AstAssignment: AstStatement {
  AstAssignment() {}
  AstAssignment(std::shared_ptr<Token> t): AstStatement(t) {}

  Parameters params;
  std::shared_ptr<AstExpression> value;

  std::string toString() override {
    std::string res = "";
    res += "(";

    for (auto param : params) {
      res += param->toString();
      res += ", ";
    }

    if (params.size() > 0) {
      res.pop_back();
      res.pop_back();
    }

    return res + ") = " + value->toString() + "\n";
  }

  std::string typeToString(u_int in) {
    return "";
  }
};