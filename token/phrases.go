package token

import (
	"regexp"
)

const ()

type Phrase struct {
	Literal string
	Type    TokenType
}

var phrases = map[string]Phrase{}

func LookupPhrase(line string) (*Phrase, []string, int) {
	for matcher, phrase := range phrases {
		regex := regexp.MustCompile(matcher)
		if regex.MatchString(line) {
			args := regex.FindStringSubmatch(line)[1:]
			length := len(regex.FindString(line))

			return &phrase, args, length
		}
	}
	return nil, nil, 0
}
