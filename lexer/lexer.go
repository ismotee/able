package lexer

import (
	"able/phrase"
	"able/scope"
	"regexp"
	"strings"
)

type Lexer struct {
	Source       string
	ProtoCode    []string
	MainScope    scope.Scope
	currentScope *scope.Scope
}

func New(source string) *Lexer {
	l := &Lexer{Source: source, MainScope: scope.Scope{Depth: 0}}
	l.currentScope = &l.MainScope
	return l
}

func (l *Lexer) MarkScope() {
	lines := l.getLines()

	for _, line := range lines {
		declareMatcher := regexp.MustCompile(`(?P<scope>[#]+) (?P<name>.*)`)
		match := declareMatcher.FindStringSubmatch(line)

		if len(match) != 0 {
			declaration := groupMatch(declareMatcher, match)
			depth := len(declaration["scope"])
			if depth > l.currentScope.Depth {
				newScope := scope.Scope{OuterScope: l.currentScope, Depth: depth}
				newPhrase := phrase.Phrase{Matcher: declaration["name"], KeyWord: "IDENT", ArgNames: []string{}}

				l.currentScope.NameSpace = append(l.currentScope.NameSpace, newPhrase)
				l.currentScope.InnerScopes = append(l.currentScope.InnerScopes, newScope)
				l.currentScope = &newScope
			}
		} else {
			l.currentScope.Lines = append(l.currentScope.Lines, line)
		}
	}
}

func groupMatch(matcher *regexp.Regexp, match []string) map[string]string {
	result := make(map[string]string)
	for i, name := range matcher.SubexpNames() {
		if i != 0 && name != "" {
			result[name] = match[i]
		}
	}
	return result
}

func (l *Lexer) GenerateProto() {
	lines := l.getLines()
	phrases := phrase.GeneratePhrases()

	for _, line := range lines {
		for _, phrase := range phrases {
			if proto, found := phrase.MatchPhrase(line); found {
				l.ProtoCode = append(l.ProtoCode, proto...)
				continue
			}
		}
		l.ProtoCode = append(l.ProtoCode, "ENDL")
	}

	// remove excess ENDL before EOF
	l.ProtoCode = l.ProtoCode[:len(l.ProtoCode)-1]
	l.ProtoCode = append(l.ProtoCode, "EOF")
}

func (l *Lexer) getLines() []string {
	return strings.Split(strings.TrimSuffix(l.Source, "\n"), "\n")
}
