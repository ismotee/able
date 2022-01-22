package scope

import (
	"testing"
)

func TestResolveScopes_EmptyInput(t *testing.T) {
	input := []string{""}

	globalScope := Scope{}
	ResolveScopes(input, &globalScope)
	innerScopes := globalScope.InnerScopes

	if len(innerScopes) != 0 {
		t.Errorf("Should return empty innerScopes, got=%+v", innerScopes)
	}
}

func TestResolveScopes_OneInnerScope(t *testing.T) {
	input := []string{"# Test"}

	globalScope := Scope{}
	ResolveScopes(input, &globalScope)
	innerScopes := globalScope.InnerScopes

	if len(innerScopes) != 1 {
		t.Errorf("Should return empty innerScopes, got=%+v", innerScopes)
	}
}
