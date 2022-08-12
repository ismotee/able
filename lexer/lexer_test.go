package lexer

import (
	"able/token"
	"testing"
)

func TestNextToken(t *testing.T) {
	input := `print hello.
	
	hello is "Hello, World!".
	+-*/()1233. 123.123. 123hello.
	== = !=.
	return.`

	// already calls findStatementAssignments
	l := New(input)

	tests := []struct {
		expType     token.TokenType
		expLiteral  string
		expPosition token.TokenPosition
	}{
		{token.PRINT, "print", token.TokenPosition{Line: 1, Char: 1, Length: 5}},
		{token.IDENT, "hello", token.TokenPosition{Line: 1, Char: 7, Length: 5}},
		{token.DOT, ".", token.TokenPosition{Line: 1, Char: 12, Length: 1}},
		{token.NEW_PARAGRAPH, "\n\n", token.TokenPosition{Line: 1, Char: 13, Length: 2}},
		{token.IDENT, "hello", token.TokenPosition{Line: 3, Char: 1, Length: 5}},
		{token.IS, "is", token.TokenPosition{Line: 3, Char: 7, Length: 2}},
		{token.STRING, "Hello, World!", token.TokenPosition{Line: 3, Char: 11, Length: 13}},
		{token.DOT, ".", token.TokenPosition{Line: 3, Char: 25, Length: 1}},
		{token.NEW_LINE, "\n", token.TokenPosition{Line: 3, Char: 26, Length: 1}},
		{token.PLUS, "+", token.TokenPosition{Line: 4, Char: 1, Length: 1}},
		{token.MINUS, "-", token.TokenPosition{Line: 4, Char: 2, Length: 1}},
		{token.ASTERISK, "*", token.TokenPosition{Line: 4, Char: 3, Length: 1}},
		{token.SLASH, "/", token.TokenPosition{Line: 4, Char: 4, Length: 1}},
		{token.LPAREN, "(", token.TokenPosition{Line: 4, Char: 5, Length: 1}},
		{token.RPAREN, ")", token.TokenPosition{Line: 4, Char: 6, Length: 1}},
		{token.NUMBER, "1233", token.TokenPosition{Line: 4, Char: 7, Length: 4}},
		{token.DOT, ".", token.TokenPosition{Line: 4, Char: 11, Length: 1}},
		{token.NUMBER, "123.123", token.TokenPosition{Line: 4, Char: 13, Length: 7}},
		{token.DOT, ".", token.TokenPosition{Line: 4, Char: 20, Length: 1}},
		{token.NUMBER, "123", token.TokenPosition{Line: 4, Char: 22, Length: 3}},
		{token.IDENT, "hello", token.TokenPosition{Line: 4, Char: 25, Length: 5}},
		{token.DOT, ".", token.TokenPosition{Line: 4, Char: 30, Length: 1}},
		{token.NEW_LINE, "\n", token.TokenPosition{Line: 4, Char: 31, Length: 1}},
		{token.EQ, "==", token.TokenPosition{Line: 5, Char: 1, Length: 2}},
		{token.IS, "=", token.TokenPosition{Line: 5, Char: 4, Length: 1}},
		{token.NOT_EQ, "!=", token.TokenPosition{Line: 5, Char: 6, Length: 2}},
		{token.DOT, ".", token.TokenPosition{Line: 5, Char: 8, Length: 1}},
		{token.NEW_LINE, "\n", token.TokenPosition{Line: 5, Char: 9, Length: 1}},
		{token.RETURN, "return", token.TokenPosition{Line: 6, Char: 1, Length: 6}},
		{token.DOT, ".", token.TokenPosition{Line: 6, Char: 7, Length: 1}},
		{token.EOF, "", token.TokenPosition{Line: 6, Char: 8, Length: 0}},
	}

	for i, tt := range tests {
		tok := l.NextToken()

		if tok.Type != tt.expType {
			t.Fatalf("tests[%d]: Token type wrong. expected: %q\n got: %q\n literal token: %q", i, tt.expType, tok.Type, tok.Literal)
		}

		if tok.Literal != tt.expLiteral {
			t.Fatalf("tests[%d]: Token literal wrong. expected %q\n got: %q\n", i, tt.expLiteral, tok.Literal)
		}

		if tok.Position.Line != tt.expPosition.Line || tok.Position.Char != tt.expPosition.Char || tok.Position.Length != tt.expPosition.Length {
			t.Fatalf("tests[%d]: position details wrong for %q .\n expected: %v\n got: %v\n", i, tok.Literal, tt.expPosition, tok.Position)
		}

	}
}
