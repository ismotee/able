package token

type IdentifierType string

const (
	FUNC = "FUNCTION"
	VAR  = "VARIABLE"
)

type Identifier struct {
	Literal string
	Matcher string
	Type    IdentifierType
}
