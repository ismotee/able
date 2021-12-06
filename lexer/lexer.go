package lexer

import (
	"able/scope"
	"able/token"
	"bufio"
	"regexp"
	"strings"
)

type Lexer struct {
	input        string
	position     int
	readPosition int
	ch           byte
	lastToken    *token.Token
	tokenBuffer  []token.Token

	GlobalScope  *scope.Scope
	currentScope *scope.Scope
}

func New(input string, global, current *scope.Scope) *Lexer {
	if global == nil {
		global = scope.New(nil, token.Identifier{Literal: "GLOBAL", Matcher: `^GLOBAL\b`})
	}
	if current == nil {
		current = global
	}

	l := &Lexer{input: input, GlobalScope: global, currentScope: current}

	l.RegisterDeclarations()
	l.currentScope = current // rewind to first scope

	l.readChar()
	return l
}

func (l *Lexer) NextToken() token.Token {
	var tok token.Token

	if len(l.tokenBuffer) > 0 {
		tok = l.tokenBuffer[0]
		l.tokenBuffer = l.tokenBuffer[1:]
		l.lastToken = &tok
		return tok
	}

	l.skipWhitespace()

	if l.ch != 0 && l.ch != '#' && l.ch != '=' {
		line := strings.TrimSpace(l.getLine())
		scope := l.currentScope.FindScope(line)

		if scope != nil {
			ident := scope.ScopeIdentifier
			tok.Type = token.IDENT
			tok.Literal = ident.Literal

			if l.lastToken.Type == token.DECLARE {
				l.currentScope = scope
				l.appendDeclParameters(line+"\n", regexp.MustCompile(ident.Matcher))
			}
			l.lastToken = &tok

			// some manual setup for HEAD
			l.readPosition += len(ident.Literal) - 1
			l.readChar()

			return tok
		} else if ident := l.currentScope.FindIdentifier(line + "\n"); ident != nil {
			tok.Type = token.IDENT
			tok.Literal = ident.Literal

			matcher := regexp.MustCompile(ident.Matcher)
			literal := matcher.FindString(line + "\n")
			l.appendCallArgs(line+"\n", matcher)

			l.lastToken = &tok

			// some manual setup for HEAD
			l.readPosition += len(literal) - 1
			l.readChar()

			return tok
		}
	}

	switch l.ch {
	case '=':
		if l.isNextChar('=') {
			literal := l.input[l.position : l.readPosition+1]
			tok = token.Token{Type: token.EQ, Literal: literal}
			l.readChar()
		} else if l.lastToken != nil && l.lastToken.Type == token.IDENT {
			tok = newToken(token.ASSIGN, l.ch)
		} else {
			tok = newToken(token.RETURN, l.ch)
		}
	case '"':
		tok.Type = token.STRING
		tok.Literal = l.readString()
	case '+':
		tok = newToken(token.PLUS, l.ch)
	case '-':
		tok = newToken(token.MINUS, l.ch)
	case '*':
		tok = newToken(token.ASTERISK, l.ch)
	case '/':
		tok = newToken(token.SLASH, l.ch)
	case '(':
		tok = newToken(token.LPAREN, l.ch)
	case ')':
		tok = newToken(token.RPAREN, l.ch)
	case '<':
		tok = newToken(token.LT, l.ch)
	case '>':
		tok = newToken(token.GT, l.ch)
	case '!':
		if l.isNextChar('=') {
			literal := l.input[l.position : l.readPosition+1]
			tok = token.Token{Type: token.NOT_EQ, Literal: literal}
			l.readChar()
		} else {
			tok = newToken(token.BANG, l.ch)
		}
	case '\n':
		tok = newToken(token.ENDL, l.ch)
	case '#':
		tok = newToken(token.DECLARE, l.ch)

	case 0:
		tok.Literal = ""
		tok.Type = token.EOF
	default:
		if l.isLegalIdentifierChar() {
			firstPos := l.position

			for l.isLegalIdentifierChar() || l.ch == ' ' {
				l.readChar()
			}
			identStr := strings.TrimSpace(l.input[firstPos:l.position])

			if l.ch == '=' && l.PeekChar() != '=' {
				tok.Literal = identStr
				tok.Type = token.IDENT
				l.currentScope.Identifiers = append(l.currentScope.Identifiers, token.Identifier{Literal: identStr, Matcher: "^" + identStr + "\\b"})
				return tok
			} else {
				splitted := strings.Split(identStr, " ")
				firstWord := splitted[0]
				splitted = splitted[1:]

				if tokenType := token.LookupIdent(firstWord); tokenType != token.IDENT {
					tok.Type = tokenType
					tok.Literal = firstWord
				} else {
					return token.Token{Type: token.ILLEGAL, Literal: firstWord}
				}

				for len(splitted) > 0 {
					if ident := l.currentScope.FindIdentifier(strings.Join(splitted, " ")); ident != nil {
						newToken := token.Token{Type: token.IDENT, Literal: ident.Literal}
						l.tokenBuffer = append(l.tokenBuffer, newToken)
						return tok
					}

					word := splitted[0]
					splitted = splitted[1:]

					if tokenType := token.LookupIdent(word); tokenType != token.IDENT {
						newToken := token.Token{Type: tokenType, Literal: word}
						l.tokenBuffer = append(l.tokenBuffer, newToken)
					} else {
						return token.Token{Type: token.ILLEGAL, Literal: word}
					}
				}

				return tok
			}

		} else if isDigit(l.ch) {
			tok.Type = token.NUMBER
			tok.Literal = l.readNumber()
			l.lastToken = &tok
			return tok
		} else {
			tok = newToken(token.ILLEGAL, l.ch)
		}
	}

	l.readChar()
	l.lastToken = &tok
	return tok
}

func (l *Lexer) RegisterDeclarations() {
	lines, _ := stringToLines(l.input)
	for _, line := range lines {
		pos := omitWhitespace(line, 0)

		if pos < len(line) && line[pos] == '#' {
			l.registerDeclaration(line, pos)
		}
	}
}

func (l *Lexer) readChar() {
	if l.readPosition >= len(l.input) {
		l.ch = 0
	} else {
		l.ch = l.input[l.readPosition]
	}
	l.position = l.readPosition
	l.readPosition += 1
}

func (l *Lexer) PeekChar() byte {
	if l.readPosition >= len(l.input) {
		return 0
	}
	return l.input[l.readPosition]
}

func (l *Lexer) registerDeclaration(line string, pos int) {
	scopeDepth := 0
	char := line[pos]
	for char == '#' {
		pos += 1
		scopeDepth += 1
		char = line[pos]
	}

	for scopeDepth <= l.currentScope.Depth {
		l.currentScope = l.currentScope.OuterScope
	}

	identifier := token.Identifier{}
	identifier.Literal = strings.TrimSpace(line[pos:])
	identifier.Matcher = createMatcher(identifier.Literal)

	findArgsExp := regexp.MustCompile(`\((.*?)\)`)
	argLiterals := findArgsExp.FindAllStringSubmatch(identifier.Matcher, -1)

	args := []token.Identifier{}
	for _, argLiteral := range argLiterals {
		identifier.Matcher = strings.Replace(identifier.Matcher, argLiteral[0], `(.+)`, 1)
		args = append(args, token.Identifier{Matcher: "^" + argLiteral[1] + "\\b", Literal: argLiteral[1]})
	}

	newScope := scope.New(l.currentScope, identifier)
	newScope.Identifiers = args
	l.currentScope.InnerScopes = append(l.currentScope.InnerScopes, *newScope)
	l.currentScope.Identifiers = append(l.currentScope.Identifiers, identifier)
	l.currentScope = newScope
}

func (l *Lexer) appendDeclParameters(line string, matcher *regexp.Regexp) {
	result := matcher.FindAllStringSubmatch(line, -1)
	if len(result) == 0 {
		l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.DECL_END, Literal: ""})
		return
	}

	args := result[0][1:]

	for _, arg := range args {
		l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.IDENT, Literal: arg[1 : len(arg)-1]})
	}
	l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.DECL_END, Literal: ""})
}

func (l *Lexer) appendCallArgs(line string, matcher *regexp.Regexp) {
	callArgs := matcher.FindAllStringSubmatch(line, -1)[0][1:]

	if len(callArgs) > 0 {
		l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.CALL, Literal: strings.TrimSpace(line)})

		for i, arg := range callArgs {
			argLexer := New(arg, l.GlobalScope, l.currentScope)
			argToken := argLexer.NextToken()

			for argToken.Type != token.EOF && argToken.Type != token.ENDL {
				l.tokenBuffer = append(l.tokenBuffer, argToken)
				argToken = argLexer.NextToken()
			}

			if i != len(callArgs)-1 {
				l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.ARG_END, Literal: ""})
			}
		}
		l.tokenBuffer = append(l.tokenBuffer, token.Token{Type: token.CALL_END, Literal: ""})
	}
}

func (l *Lexer) skipWhitespace() {
	for l.ch == ' ' || l.ch == '\t' {
		l.readChar()
	}
}

func (l *Lexer) getLine() string {
	pos := l.position

	for pos < len(l.input) && !lineEnd(l.input[pos]) {
		pos += 1
	}
	return l.input[l.position:pos]
}

func (l *Lexer) readNumber() string {
	position := l.position
	for isDigit(l.ch) {
		l.readChar()
	}
	numberStr := l.input[position:l.position]
	return numberStr
}

func (l *Lexer) isNextChar(ch byte) bool {
	return l.readPosition < len(l.input) && l.input[l.readPosition] == ch
}

func (l *Lexer) isLegalIdentifierChar() bool {
	return 'a' <= l.ch && l.ch <= 'z' ||
		'A' <= l.ch && l.ch <= 'Z'
}

func lineEnd(ch byte) bool {
	return ch == '\n' || ch == '\r'
}

func newToken(tokenType token.TokenType, ch byte) token.Token {
	return token.Token{Type: tokenType, Literal: string(ch)}
}

func stringToLines(s string) (lines []string, err error) {
	scanner := bufio.NewScanner(strings.NewReader(s))
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	err = scanner.Err()
	return
}

func omitWhitespace(str string, pos int) int {
	for pos < len(str) && (str[pos] == ' ' || str[pos] == '\t') {
		pos += 1
	}
	return pos
}

func createMatcher(literal string) string {
	trimmed := strings.TrimSpace(literal)
	start := "^"
	end := `\s`
	return start + trimmed + end
}

func isDigit(ch byte) bool {
	return '0' <= ch && ch <= '9' || ch == '.'
}

func (l *Lexer) readString() string {
	position := l.position + 1
	for {
		l.readChar()
		if l.ch == '"' || l.ch == 0 {
			break
		}
	}
	return l.input[position:l.position]
}
