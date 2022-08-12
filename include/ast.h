#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "token.h"

struct AstNode;
typedef std::shared_ptr<AstNode> pAstNode;
typedef std::vector<pAstNode> SemanticTokens; 

struct AstNode
{
  AstNode(std::shared_ptr<Token> t = nullptr) : token(t) {}
  virtual std::string tokenLiteral()
  {
    return token->literal;
  }
  virtual std::string toString() = 0;

  std::shared_ptr<Token> token;
};

struct AstStatement : AstNode
{
  AstStatement(std::shared_ptr<Token> t = nullptr) : AstNode(t) {}
};

struct AstExpression : AstNode
{
  AstExpression(std::shared_ptr<Token> t = nullptr) : AstNode(t) {}
};

typedef std::vector<std::shared_ptr<AstStatement>> Statements;

struct AstProgram : AstNode
{
  Statements statements;

  std::string tokenLiteral() override
  {
    if (statements.size() > 0)
    {
      return statements[0]->tokenLiteral();
    }
    else
    {
      return "";
    }
  }

  std::string toString()
  {
    std::string out;
    for (auto stmt : statements)
    {
      out += stmt->toString();
    }
    return out;
  }
};

struct AstNumber : AstExpression
{
  AstNumber() {}
  AstNumber(pToken t, double v) : AstExpression(t), value(v) {}

  double value;

  std::string toString()
  {
    std::string nbr = std::to_string(value);
    for (auto it = nbr.end() - 1; (*it) == '0' || (*it) == '.'; --it)
    {
      nbr.erase(it);
    }
    return nbr;
  }
};

struct AstExpressionStatement : AstStatement
{
  AstExpressionStatement() {}
  AstExpressionStatement(std::shared_ptr<Token> t, std::shared_ptr<AstExpression> e) : AstStatement(t), expression(e) {}

  std::shared_ptr<AstExpression> expression;

  std::string toString()
  {
    if (expression != nullptr)
    {
      return expression->toString() + "\n";
    }
    return "";
  }
};

struct AstPrefixExpression : AstExpression
{
  AstPrefixExpression() : AstExpression() {}
  AstPrefixExpression(std::shared_ptr<Token> t, std::shared_ptr<AstExpression> e) : AstExpression(t), right(e) {}

  // token is same as operator
  std::shared_ptr<AstExpression> right;

  std::string toString()
  {
    return "(" + token->literal + right->toString() + ")";
  }
};

struct AstInfixExpression : AstExpression
{
  AstInfixExpression() {}
  AstInfixExpression(
      std::shared_ptr<Token> t,
      std::shared_ptr<AstExpression> l = nullptr,
      std::shared_ptr<AstExpression> r = nullptr) : AstExpression(t),
                                                    left(l),
                                                    right(r) {}

  std::shared_ptr<AstExpression> left;
  std::shared_ptr<AstExpression> right;

  std::string toString()
  {
    return "(" + left->toString() + " " + token->literal + " " + right->toString() + ")";
  }
};

struct AstIdentifier
{
  AstIdentifier() {}

  Tokens identifier;

  std::string toString()
  {
    std::string res;
    for (auto part : identifier)
    {
      res = res + part->literal + " ";
    }
    res.pop_back();
    return res;
  }

  bool compare(Tokens::iterator it)
  {
    for (auto token : identifier)
    {
      if ((*it)->isTypeOf({TokenType::ENDL, TokenType::END_OF_FILE}))
      {
        return false;
      }
      if (token->literal == (*it)->literal && token->type == (*it)->type)
      {
        ++it;
      }
      else
      {
        return false;
      }
    }
    return true;
  }
};

struct AstParameter : AstExpression, AstIdentifier
{
  AstParameter() {}
  AstParameter(std::shared_ptr<Token> t) : AstExpression(t) {}

  std::string toString() override { return AstIdentifier::toString(); }
};

typedef std::vector<std::shared_ptr<AstParameter>> Parameters;

struct AstDeclarationStatement : AstStatement, AstIdentifier
{
  AstDeclarationStatement() {}
  AstDeclarationStatement(std::shared_ptr<Token> t) : AstStatement(t) {}

  Parameters params;
  Statements block;

  std::string toString() override
  {
    std::string res = token->literal + " ";

    res += AstIdentifier::toString();
    res += "(";

    for (auto param : params)
    {
      res += param->toString();
      res += ", ";
    }
    if (params.size() > 0)
    {
      res.pop_back();
      res.pop_back();
    }

    res += ") {\n";
    for (auto stmt : block)
    {
      res += stmt->toString();
    }
    res += "}\n";
    return res;
  }
};

struct AstAssignment : AstStatement, AstIdentifier
{
  AstAssignment() {}
  AstAssignment(std::shared_ptr<Token> t) : AstStatement(t) {}

  Parameters params;
  std::shared_ptr<AstExpression> value;

  std::string toString() override
  {
    std::string res = AstIdentifier::toString();
    res += "(";

    for (auto param : params)
    {
      res += param->toString();
      res += ", ";
    }

    if (params.size() > 0)
    {
      res.pop_back();
      res.pop_back();
    }

    return res + ") = " + value->toString() + "\n";
  }
};

struct AstCallExpression : AstExpression, AstIdentifier
{
  AstCallExpression() {}
  AstCallExpression(std::shared_ptr<Token> t) : AstExpression(t) {}

  std::vector<std::shared_ptr<AstExpression>> arguments;

  std::string toString() override
  {
    std::string res = "{";
    res += AstIdentifier::toString();

    for (auto arg : arguments)
    {
      res += arg->toString();
    }

    res += "}";
    return res;
  }
};
