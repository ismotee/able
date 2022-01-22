package lexer

import (
	"able/phrase"
	"testing"
)

func TestProtoCode(t *testing.T) {
	input := `print 3
	for each item in list
	modify data while true`
	expectedProtos := []string{
		"PRINT",
		"ARG 3",
		"ENDL",
		"FOR EACH",
		"ARG item",
		"ARG list",
		"ENDL",
		"WHILE",
		"DO modify data",
		"COND true",
		"EOF",
	}

	l := New(input)
	l.GenerateProto()

	if len(l.ProtoCode) != len(expectedProtos) {
		t.Fatalf("Length of ProtoCode should be %d, got=%d", len(expectedProtos), len(l.ProtoCode))
	}

	for i, expectedProto := range expectedProtos {
		actualProto := l.ProtoCode[i]
		if expectedProto != actualProto {
			t.Errorf("ProtoLine  should be %s, got=%s", expectedProto, actualProto)
		}
	}
}

func TestProtoInnerScope(t *testing.T) {
	input := `# Main program
	other line`

	expectedScopes := []struct {
		name      string
		lines     []string
		NameSpace []string
	}{
		{name: "Main program", lines: []string{"other line"}},
	}

	l := New(input)
	l.MarkScope()

	if len(l.MainScope.InnerScopes) != len(expectedScopes) {
		t.Fatalf("Length of InnerScopes should be %d, got=%d", len(expectedScopes), len(l.MainScope.InnerScopes))
	}

	for _, expectedScope := range expectedScopes {
		if !checkForMatcherInNameSpace(expectedScope.name, l.MainScope.NameSpace) {
			t.Errorf("Should Find Matcher '%s'", expectedScope.name)
		}

		// check lines but first convert InnerScopes to map in order to find proper scope

	}
}

func checkForMatcherInNameSpace(expectedName string, nameSpace []phrase.Phrase) bool {
	for _, name := range nameSpace {
		if name.Matcher == expectedName {
			return true
		}
	}
	return false
}
