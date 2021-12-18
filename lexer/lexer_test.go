package lexer

import (
	"able/token"
	"fmt"
	"testing"
)

// onelinereitä ei tällä hetkellä tueta muttei tarkastetakaan
// # funktio = jotain
// myös kaikki erikoismerkit kuten !=<> on sallittuja. Ei varmaan pitäis.. :D
// Jatkossa ehkä tyyliin add (x) to (y) = x + y
// oneliner syntaksina.

// Scope on täysin testaamatta. Kirjoita testit

func TestNextToken(t *testing.T) {
	input := `# add (x) to (value y)
	x + value y
	value y - 12
	# other function
 	test = 2
	true < false
	if false > !true 
	else test

	12.33323 + 12
	true == true
	!=
	add 1 to 10
	1`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.DECLARE, "#"},
		{token.IDENT, "add (x) to (value y)"},
		{token.IDENT, "x"},
		{token.IDENT, "value y"},
		{token.DECL_END, ""},
		{token.ENDL, "\n"},
		{token.IDENT, "x"},
		{token.PLUS, "+"},
		{token.IDENT, "value y"},
		{token.ENDL, "\n"},
		{token.IDENT, "value y"},
		{token.MINUS, "-"},
		{token.NUMBER, "12"},
		{token.ENDL, "\n"},
		{token.DECLARE, "#"},
		{token.IDENT, "other function"},
		{token.DECL_END, ""},
		{token.ENDL, "\n"},
		{token.IDENT, "test"},
		{token.ASSIGN, "="},
		{token.NUMBER, "2"},
		{token.ENDL, "\n"},
		{token.TRUE, "true"},
		{token.LT, "<"},
		{token.FALSE, "false"},
		{token.ENDL, "\n"},
		{token.IF, "if"},
		{token.FALSE, "false"},
		{token.GT, ">"},
		{token.BANG, "!"},
		{token.TRUE, "true"},
		{token.ENDL, "\n"},
		{token.ELSE, "else"},
		{token.IDENT, "test"},
		{token.ENDL, "\n"},
		{token.ENDL, "\n"},
		{token.NUMBER, "12.33323"},
		{token.PLUS, "+"},
		{token.NUMBER, "12"},
		{token.ENDL, "\n"},
		{token.TRUE, "true"},
		{token.EQ, "=="},
		{token.TRUE, "true"},
		{token.ENDL, "\n"},
		{token.NOT_EQ, "!="},
		{token.ENDL, "\n"},
		{token.IDENT, "add (x) to (value y)"},
		{token.CALL, "add 1 to 10"},
		{token.NUMBER, "1"},
		{token.ARG_END, ""},
		{token.NUMBER, "10"},
		{token.CALL_END, ""},
		{token.NUMBER, "1"},
		{token.EOF, ""},
	}

	l := New(input, nil, nil)

	for i, tt := range tests {
		tok := l.NextToken()
		fmt.Printf("current token in test: %s, %s\n", tok.Literal, tok.Type)
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}
}

func TestNextTokenDeclaration(t *testing.T) {
	input := `add 1 to 10
	
	# add (x) to (value y)
	= value y + x`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.IDENT, "add (x) to (value y)"},
		{token.CALL, "add 1 to 10"},
		{token.NUMBER, "1"},
		{token.ARG_END, ""},
		{token.NUMBER, "10"},
		{token.CALL_END, ""},
		{token.ENDL, "\n"},
		{token.DECLARE, "#"},
		{token.IDENT, "add (x) to (value y)"},
		{token.IDENT, "x"},
		{token.IDENT, "value y"},
		{token.DECL_END, ""},
		{token.ENDL, "\n"},
		{token.RETURN, "="},
		{token.IDENT, "value y"},
		{token.PLUS, "+"},
		{token.IDENT, "x"},
		{token.EOF, ""},
	}

	l := New(input, nil, nil)

	for i, tt := range tests {
		tok := l.NextToken()
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}
}

func TestNextTokenDeclarationWithArgumentFirst(t *testing.T) {
	input := `3 apples
	
	# (x) apples
	= x - 1`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.IDENT, "(x) apples"},
		{token.CALL, "3 apples"},
		{token.NUMBER, "3"},
		{token.CALL_END, ""},
		{token.ENDL, "\n"},
		{token.DECLARE, "#"},
		{token.IDENT, "(x) apples"},
		{token.IDENT, "x"},
		{token.DECL_END, ""},
		{token.ENDL, "\n"},
		{token.RETURN, "="},
		{token.IDENT, "x"},
		{token.MINUS, "-"},
		{token.NUMBER, "1"},
		{token.EOF, ""},
	}

	l := New(input, nil, nil)
	for i, tt := range tests {
		tok := l.NextToken()
		fmt.Printf("current token in test: %s, %s\n", tok.Literal, tok.Type)
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}

}

func TestNextTokenString(t *testing.T) {
	input := `"foobar"
	"some longer text"
	"longer text (with parentheses)"`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.STRING, "foobar"},
		{token.ENDL, "\n"},
		{token.STRING, "some longer text"},
		{token.ENDL, "\n"},
		{token.STRING, "longer text (with parentheses)"},
		{token.EOF, ""},
	}

	l := New(input, nil, nil)

	for i, tt := range tests {
		tok := l.NextToken()
		fmt.Printf("current token in test: %s, %s\n", tok.Literal, tok.Type)
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}
}

func TestColonAssignment(t *testing.T) {
	input := `todo:
	123`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.IDENT, "todo"},
		{token.COLON, ":"},
		{token.ENDL, "\n"},
		{token.NUMBER, "123"},
		{token.EOF, ""},
	}

	l := New(input, nil, nil)

	for i, tt := range tests {
		tok := l.NextToken()
		fmt.Printf("current token in test: %s, %s\n", tok.Literal, tok.Type)
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}
}

func TestList(t *testing.T) {
	input := `* 123
	* "foobar"`

	tests := []struct {
		expectedType    token.TokenType
		expectedLiteral string
	}{
		{token.ASTERISK, "*"},
		{token.NUMBER, "123"},
		{token.ENDL, "\n"},
		{token.ASTERISK, "*"},
		{token.STRING, "foobar"},
	}

	l := New(input, nil, nil)

	for i, tt := range tests {
		tok := l.NextToken()
		fmt.Printf("current token in test: %s, %s\n", tok.Literal, tok.Type)
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("Test number %d: Token literal is not expected '%s', got='%s'", i, tt.expectedLiteral, tok.Literal)
		}

		if tok.Type != tt.expectedType {
			t.Fatalf("Test number %d: Token '%s' is not expected type '%s', got=%s", i, tok.Literal, tt.expectedType, tok.Type)
		}
	}
}
