package token

type TokenType string

type Token struct {
	Type    TokenType
	Literal string
}

const (
	ILLEGAL = "ILLEGAL"
	EOF     = "EOF"

	NUMBER   = "NUMBER"
	STRING   = "STRING"
	IDENT    = "IDENT"
	CALL     = "CALL"
	CALL_END = "CALL_END"
	ARG_END  = "ARG_END"

	COLON    = ":"
	ASSIGN   = "="
	RETURN   = "="
	PLUS     = "+"
	MINUS    = "-"
	SLASH    = "/"
	ASTERISK = "*"

	LT     = "<"
	GT     = ">"
	EQ     = "=="
	NOT_EQ = "!="
	BANG   = "!"

	LPAREN = "("
	RPAREN = ")"

	ENDL     = "\n"
	DECLARE  = "#"
	DECL_END = "DECL_END"

	// keywords
	IF    = "IF"
	ELSE  = "ELSE"
	TRUE  = "TRUE"
	FALSE = "FALSE"
)

var keywords = map[string]TokenType{
	"if":    IF,
	"else":  ELSE,
	"true":  TRUE,
	"false": FALSE,
}

func LookupIdent(ident string) TokenType {
	if tok, ok := keywords[ident]; ok {
		return tok
	}
	return IDENT
}
