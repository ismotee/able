package scope

import (
	"able/token"
	"regexp"
)

type Scope struct {
	Depth           int
	ScopeIdentifier token.Identifier
	OuterScope      *Scope
	InnerScopes     []*Scope

	Identifiers []token.Identifier
}

func New(outerScope *Scope, ident token.Identifier, depth int) *Scope {
	if outerScope != nil {
		return &Scope{Depth: depth, OuterScope: outerScope, ScopeIdentifier: ident}
	}
	return &Scope{Depth: 0}
}

func (s *Scope) FindIdentifier(term string) (*token.Identifier, int) {
	for _, ident := range s.Identifiers {
		matcher := regexp.MustCompile(ident.Matcher)
		if matcher.MatchString(term) {
			rng := matcher.FindStringIndex(term)
			return &ident, rng[1] - rng[0]
		}
	}

	if s.OuterScope != nil {
		return s.OuterScope.FindIdentifier(term)
	}

	return nil, 0
}

func (s *Scope) FindScope(term string) *Scope {
	for _, innerScope := range s.InnerScopes {
		if term == innerScope.ScopeIdentifier.Literal {
			return innerScope
		}
	}

	if s.OuterScope != nil {
		return s.OuterScope.FindScope(term)
	}

	return nil
}
