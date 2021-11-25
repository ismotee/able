package scope

import (
	"able/token"
	"regexp"
)

type Scope struct {
	Depth           int
	ScopeIdentifier token.Identifier
	OuterScope      *Scope
	InnerScopes     []Scope

	Identifiers []token.Identifier
}

func New(outerScope *Scope, ident token.Identifier) *Scope {
	if outerScope != nil {
		return &Scope{Depth: outerScope.Depth + 1, OuterScope: outerScope, ScopeIdentifier: ident}
	}
	return &Scope{Depth: 0}
}

func (s *Scope) FindIdentifier(term string) *token.Identifier {
	for _, ident := range s.Identifiers {
		matcher := regexp.MustCompile(ident.Matcher)
		if matcher.MatchString(term) {
			return &ident
		}
	}

	if s.OuterScope != nil {
		return s.OuterScope.FindIdentifier(term)
	}

	return nil
}

func (s *Scope) FindScope(term string) *Scope {
	for _, innerScope := range s.InnerScopes {
		if term == innerScope.ScopeIdentifier.Literal {
			return &innerScope
		}
	}

	if s.OuterScope != nil {
		return s.OuterScope.FindScope(term)
	}

	return nil
}
