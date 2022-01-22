package parser

import (
	"able/ast"
	"able/lexer"
	"able/token"
	"strconv"
)

const (
	_ = iota
	LOWEST
	ASSIGN
	CONDITION
	EQUALS
	LESSGREATER
	SUM
	PRODUCT
	PREFIX
	CALL
)

var precedences = map[token.TokenType]int{
	token.ASSIGN:   ASSIGN,
	token.IF:       CONDITION,
	token.ELSE:     CONDITION,
	token.EQ:       EQUALS,
	token.NOT_EQ:   EQUALS,
	token.LT:       LESSGREATER,
	token.GT:       LESSGREATER,
	token.PLUS:     SUM,
	token.MINUS:    SUM,
	token.SLASH:    PRODUCT,
	token.ASTERISK: PRODUCT,
	token.CALL:     CALL,
	token.CALL_END: CALL,
}

type Parser struct {
	l *lexer.Lexer

	curToken  token.Token
	peekToken token.Token

	prefixParseFns map[token.TokenType]prefixParseFn
	infixParseFns  map[token.TokenType]infixParseFn
}

type (
	prefixParseFn func() ast.Expression
	infixParseFn  func(ast.Expression) ast.Expression
)

func (p *Parser) registerPrefix(tokenType token.TokenType, fn prefixParseFn) {
	p.prefixParseFns[tokenType] = fn
}

func (p *Parser) registerInfix(tokenType token.TokenType, fn infixParseFn) {
	p.infixParseFns[tokenType] = fn
}

func New(l *lexer.Lexer) *Parser {
	p := &Parser{l: l}

	p.prefixParseFns = make(map[token.TokenType]prefixParseFn)
	p.registerPrefix(token.IDENT, p.parseIdentifier)
	p.registerPrefix(token.NUMBER, p.parseNumberLiteral)
	p.registerPrefix(token.BANG, p.parsePrefixExpression)
	p.registerPrefix(token.MINUS, p.parsePrefixExpression)
	p.registerPrefix(token.ASTERISK, p.parseListExpression)
	p.registerPrefix(token.TRUE, p.parseBoolean)
	p.registerPrefix(token.FALSE, p.parseBoolean)
	p.registerPrefix(token.LPAREN, p.parseGroupedExpression)
	p.registerPrefix(token.STRING, p.parseStringLiteral)
	p.registerPrefix(token.CALL, p.parseCallExpression)

	p.infixParseFns = make(map[token.TokenType]infixParseFn)
	p.registerInfix(token.PLUS, p.parseInfixExpression)
	p.registerInfix(token.MINUS, p.parseInfixExpression)
	p.registerInfix(token.SLASH, p.parseInfixExpression)
	p.registerInfix(token.ASTERISK, p.parseInfixExpression)
	p.registerInfix(token.EQ, p.parseInfixExpression)
	p.registerInfix(token.NOT_EQ, p.parseInfixExpression)
	p.registerInfix(token.LT, p.parseInfixExpression)
	p.registerInfix(token.GT, p.parseInfixExpression)
	p.registerInfix(token.IF, p.parseIfExpression)
	p.registerInfix(token.ASSIGN, p.parseAssignExpression)

	p.nextToken()
	p.nextToken()

	return p
}

func (p *Parser) nextToken() {
	p.curToken = p.peekToken
	p.peekToken = p.l.NextToken()
}

func (p *Parser) ParseProgram() *ast.Program {
	program := &ast.Program{Statements: []ast.Statement{}}

	for p.curToken.Type != token.EOF {
		stmt := p.parseStatement()
		if stmt != nil {
			program.Statements = append(program.Statements, stmt)
		}
		p.nextToken()
	}

	return program
}

func (p *Parser) parseStatement() ast.Statement {
	switch p.curToken.Type {
	case token.DECLARE:
		return p.parseDeclaration()
	case token.RETURN:
		return p.parseReturnStatement()
	case token.ENDL:
		return nil
	default:
		return p.parseExpressionStatement()
	}
}

func (p *Parser) parseDeclaration() *ast.Declaration {
	stmt := &ast.Declaration{Token: p.curToken}

	depth := 1

	for p.curToken.Type == token.DECLARE {
		depth += 1
		p.nextToken()
	}

	// todo: make sure next token is identifier

	stmt.Name = &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
	stmt.Parameters = p.parseDeclarationParams()
	stmt.Body = p.parseBlockStatement()
	return stmt
}

func (p *Parser) parseDeclarationParams() []*ast.Identifier {
	params := []*ast.Identifier{}
	p.nextToken()

	for p.curToken.Type != token.DECL_END {
		params = append(params, &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal})
		p.nextToken()
	}

	return params
}

func (p *Parser) parseBlockStatement() *ast.BlockStatement {
	block := &ast.BlockStatement{Token: p.curToken}
	block.Statements = []ast.Statement{}

	p.nextToken()

	for !p.curTokenIs(token.EOF) && !p.peekTokenIs(token.DECLARE) {
		stmt := p.parseStatement()
		if stmt != nil {
			block.Statements = append(block.Statements, stmt)
		}
		p.nextToken()
	}
	return block
}

func (p *Parser) parseReturnStatement() *ast.ReturnStatement {
	stmt := &ast.ReturnStatement{Token: p.curToken}

	p.nextToken()

	stmt.ReturnValue = p.parseExpression(LOWEST)

	return stmt
}

func (p *Parser) parseExpressionStatement() *ast.ExpressionStatement {
	stmt := &ast.ExpressionStatement{Token: p.curToken}

	stmt.Expression = p.parseExpression(LOWEST)

	return stmt
}

func (p *Parser) parseExpression(precedence int) ast.Expression {
	prefix := p.prefixParseFns[p.curToken.Type]
	if prefix == nil {
		// register error
		return nil
	}
	leftExp := prefix()

	for precedence < p.peekPrecedence() {
		infix := p.infixParseFns[p.peekToken.Type]
		if infix == nil {
			return leftExp
		}

		p.nextToken()

		leftExp = infix(leftExp)
	}

	return leftExp
}

func (p *Parser) parseIdentifier() ast.Expression {
	return &ast.Identifier{Token: p.curToken, Value: p.curToken.Literal}
}

func (p *Parser) parseNumberLiteral() ast.Expression {
	lit := &ast.NumberLiteral{Token: p.curToken}

	value, err := strconv.ParseFloat(p.curToken.Literal, 64)
	if err != nil {
		// handle error
		return nil
	}

	lit.Value = value

	return lit
}

func (p *Parser) parseBoolean() ast.Expression {
	expression := &ast.Boolean{Token: p.curToken, Value: p.curTokenIs(token.TRUE)}
	return expression
}

func (p *Parser) parseGroupedExpression() ast.Expression {
	p.nextToken()

	exp := p.parseExpression(LOWEST)

	if !p.expectPeek(token.RPAREN) {
		return nil
	}

	return exp
}

func (p *Parser) parseStringLiteral() ast.Expression {
	return &ast.StringLiteral{Token: p.curToken, Value: p.curToken.Literal}
}

func (p *Parser) parseListExpression() ast.Expression {
	expression := &ast.ListExpression{Token: p.curToken}

	for p.curToken.Type == token.ASTERISK {
		expression.Elements = append(expression.Elements, p.parseListItemExpression())
		p.nextToken()
	}

	return expression
}

func (p *Parser) parseListItemExpression() ast.Expression {
	p.nextToken()
	expression := p.parseExpression(LOWEST)
	p.ignoreToken(token.ENDL)

	return expression
}

func (p *Parser) parsePrefixExpression() ast.Expression {
	expression := &ast.PrefixExpression{Token: p.curToken, Operator: p.curToken.Literal}

	p.nextToken()

	expression.Right = p.parseExpression(PREFIX)

	return expression
}

func (p *Parser) parseInfixExpression(left ast.Expression) ast.Expression {
	expression := &ast.InfixExpression{Token: p.curToken, Operator: p.curToken.Literal, Left: left}

	precedence := p.curPrecedence()
	p.nextToken()
	expression.Right = p.parseExpression(precedence)

	return expression
}

func (p *Parser) parseAssignExpression(left ast.Expression) ast.Expression {
	expression := &ast.AssignExpression{Token: p.curToken, Name: left.(*ast.Identifier)}

	precedence := p.curPrecedence()
	p.ignoreToken(token.ENDL)
	p.nextToken()
	expression.Value = p.parseExpression(precedence)

	return expression
}

func (p *Parser) parseIfExpression(left ast.Expression) ast.Expression {
	expression := &ast.IfExpression{Token: p.curToken, Consequence: left}
	p.nextToken()
	expression.Condition = p.parseExpression(LOWEST)

	if p.expectPeek(token.ELSE) {
		p.nextToken()
		expression.Alternative = p.parseExpression(LOWEST)
	}

	return expression
}

func (p *Parser) parseCallExpression() ast.Expression {
	exp := &ast.CallExpression{Token: p.curToken, Function: p.parseIdentifier()}
	exp.Arguments = p.parseCallArguments()

	return exp
}

func (p *Parser) parseCallArguments() []ast.Expression {
	args := []ast.Expression{}
	p.nextToken()

	for !p.curTokenIs(token.CALL_END) {
		args = append(args, p.parseExpression(LOWEST))
		p.nextToken()
		if p.curToken.Type == token.ARG_END {
			p.nextToken()
		}
	}
	return args
}

// TODO: expect peek error missing

func (p *Parser) expectPeek(t token.TokenType) bool {
	if p.peekTokenIs(t) {
		p.nextToken()
		return true
	}
	return false
}

func (p *Parser) curTokenIs(t token.TokenType) bool {
	return p.curToken.Type == t
}

func (p *Parser) peekTokenIs(t token.TokenType) bool {
	return p.peekToken.Type == t
}

func (p *Parser) peekPrecedence() int {
	if p, ok := precedences[p.peekToken.Type]; ok {
		return p
	}

	return LOWEST
}

func (p *Parser) curPrecedence() int {
	if p, ok := precedences[p.curToken.Type]; ok {
		return p
	}

	return LOWEST
}

func (p *Parser) ignoreToken(tok token.TokenType) {
	for p.peekTokenIs(tok) {
		p.nextToken()
	} // Just skip this type of tokens
}
