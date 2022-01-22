package scope

import "able/phrase"

type Scope struct {
	Lines       []string
	NameSpace   []phrase.Phrase
	OuterScope  *Scope
	InnerScopes []Scope
	Depth       int
}
