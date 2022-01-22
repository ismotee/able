package scope

import (
	"able/phrase"
	"regexp"
)

type Scope struct {
	Lines       []string
	NameSpace   []phrase.Phrase
	OuterScope  *Scope
	InnerScopes []Scope
	Depth       int
}

func ResolveScopes(lines []string, currentScope *Scope) {
	for i, line := range lines {
		declareMatcher := regexp.MustCompile(`(?P<scope>[#]+) (?P<name>.*)`)
		match := declareMatcher.FindStringSubmatch(line)

		if len(match) != 0 {
			declaration := groupMatch(declareMatcher, match)
			depth := len(declaration["scope"])
			if depth > currentScope.Depth {
				newScope := Scope{OuterScope: currentScope, Depth: depth}
				newPhrase := phrase.Phrase{Matcher: declaration["name"], KeyWord: "IDENT", ArgNames: []string{}}

				currentScope.NameSpace = append(currentScope.NameSpace, newPhrase)
				currentScope.InnerScopes = append(currentScope.InnerScopes, newScope)
				ResolveScopes(lines[:i], &newScope)
			}
		} else {
			currentScope.Lines = append(currentScope.Lines, line)
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
