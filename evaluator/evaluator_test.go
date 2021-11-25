package evaluator

import (
	"able/lexer"
	"able/object"
	"able/parser"
	"testing"
)

func TestNumberExpression(t *testing.T) {
	tests := []struct {
		input    string
		expected float64
	}{
		{"5", 5.0},
		{"10.2", 10.2},
		{"-5", -5.0},
		{"-10.2", -10.2},
		{"5 + 5 + 5 + 5 - 10", 10},
		{"2 * 2 * 2 * 2 * 2", 32},
		{"-50 + 100  + -50", 0},
		{"5 * 2 + 10", 20},
		{"5 + 2 * 10", 25},
		{"20 + 2 * -10", 0},
		{"50 / 2 * 2 + 10", 60},
		{"2 * (5 + 10)", 30},
		{"3 * 3 * 3 + 10", 37},
		{"3 * (3 * 3) + 10", 37},
		{"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testNumberObject(t, evaluated, tt.expected)
	}
}

func TestBooleanExpression(t *testing.T) {
	tests := []struct {
		input    string
		expected bool
	}{
		{"true", true},
		{"false", false},
		{"1 < 2", true},
		{"1 > 2", false},
		{"1 < 1", false},
		{"1 > 1", false},
		{"1 == 1", true},
		{"1 != 1", false},
		{"1 == 2", false},
		{"1 != 2", true},
		{"true == true", true},
		{"false != false", false},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testBooleanObject(t, evaluated, tt.expected)
	}

}

func TestStringLiteral(t *testing.T) {
	input := `"Hello, World!"`

	evaluated := testEval(input)
	str, ok := evaluated.(*object.String)
	if !ok {
		t.Errorf("Object is not String, got=%T (%+v)", evaluated, evaluated)
	}

	if str.Value != "Hello, World!" {
		t.Errorf("String has wrong value. got=%s", str.Value)
	}
}

func TestStringConcat(t *testing.T) {
	input := `"foo" + "bar"`
	evaluated := testEval(input)
	str, ok := evaluated.(*object.String)
	if !ok {
		t.Errorf("Object is not String, got=%T (%+v)", evaluated, evaluated)
	}

	if str.Value != "foobar" {
		t.Errorf("String has wrong value. got=%s", str.Value)
	}
}

func TestBangOperator(t *testing.T) {
	tests := []struct {
		input    string
		expected bool
	}{
		{"!true", false},
		{"!false", true},
		{"!5", false},
		{"!!true", true},
		{"!!false", false},
		{"!!-5", true},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testBooleanObject(t, evaluated, tt.expected)
	}
}

func TestIfElseExpression(t *testing.T) {
	tests := []struct {
		input    string
		expected interface{}
	}{
		{"10 if true", 10.0},
		{"10 if false", nil},
		{"10 if 1", 10.0},
		{"10 if 1 < 2", 10.0},
		{"10 if 1 > 2", nil},
		{"10 if 1 > 2 else 20", 20.0},
		{"10 if 1 < 2 else 20", 10.0},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		number, ok := tt.expected.(float64)
		if ok {
			testNumberObject(t, evaluated, number)
		} else {
			testNullObject(t, evaluated)
		}
	}
}

func TestReturnStatements(t *testing.T) {
	tests := []struct {
		input    string
		expected float64
	}{
		{"= 10", 10.0},
		{"= 10\n9", 10.0},
		{"= 2 * 5\n9", 10.0},
		{"9\n = 2 * 5\n 9", 10.0},
		{"= 1 if 1 > 10 else 10", 10.0},
		{"= 1 if 1 < 10 else 10", 1.0},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testNumberObject(t, evaluated, tt.expected)
	}
}

func TestAssignStatements(t *testing.T) {
	tests := []struct {
		input    string
		expected float64
	}{
		{"a = 10\na", 10.0},
		{`
		A very long identifier = 10
		A very long identifier`,
			10.0},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testNumberObject(t, evaluated, tt.expected)
	}
}

func TestDeclarationsAndCalls(t *testing.T) {
	tests := []struct {
		input    string
		expected float64
	}{
		{`minus 10 and 1 and so on

		# minus (x) and (y) and so on
			= x - y`, 9.0},
	}

	for _, tt := range tests {
		evaluated := testEval(tt.input)
		testNumberObject(t, evaluated, tt.expected)
	}

}

func testEval(input string) object.Object {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()
	env := object.NewEnvironment()

	return Eval(program, env)
}

func testNumberObject(t *testing.T, obj object.Object, expected float64) bool {
	result, ok := obj.(*object.Number)
	if !ok {
		t.Errorf("object is not Number, got=%T (%+v)", obj, obj)
		return false
	}
	if result.Value != expected {
		t.Errorf("object has wrong Value expected=%f, got=%f", expected, result.Value)
		return false
	}
	return true
}

func testBooleanObject(t *testing.T, obj object.Object, expected bool) bool {
	result, ok := obj.(*object.Boolean)
	if !ok {
		t.Errorf("object is not Boolean, got=%T (%+v)", obj, obj)
		return false
	}
	if result.Value != expected {
		t.Errorf("object has wrong Value expected=%t, got=%t", expected, result.Value)
		return false
	}
	return true
}

func testNullObject(t *testing.T, obj object.Object) bool {
	if obj != NULL {
		t.Errorf("Object not null, got=%T (%+v)", obj, obj)
		return false
	}
	return true
}
