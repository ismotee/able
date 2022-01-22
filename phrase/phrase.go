package phrase

import (
	"regexp"
	"strings"
)

type Phrase struct {
	Matcher  string
	KeyWord  string
	ArgNames []string
}

func (p *Phrase) MatchPhrase(line string) ([]string, bool) {
	protoCode := []string{}
	matcher := regexp.MustCompile(p.Matcher)
	if matcher.MatchString(line) {
		protoCode = append(protoCode, p.KeyWord)
		args := matcher.FindStringSubmatch(line)[1:]
		for i, arg := range args {
			protoCode = append(protoCode, p.ArgNames[i]+" "+strings.TrimSpace(arg))
		}
	}
	return protoCode, len(protoCode) > 0
}

func GeneratePhrases() []Phrase {
	return []Phrase{
		{Matcher: "print (.+)", KeyWord: "PRINT", ArgNames: []string{"ARG"}},
		{Matcher: "for each (.+) in (.+)", KeyWord: "FOR EACH", ArgNames: []string{"ARG", "ARG"}},
		{Matcher: "(.+) while (.+)", KeyWord: "WHILE", ArgNames: []string{"DO", "COND"}},
	}
}
