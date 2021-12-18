package parser

import (
	"able/ast"
	"able/lexer"
	"fmt"
	"strconv"
	"testing"
)

type BodyStmt struct {
	left     interface{}
	operator string
	right    interface{}
}

func TestDeclaration(t *testing.T) {
	input := `# function (x) and (y)
	x + y

	# other function
	1 * 2`

	tests := []struct {
		expectedIdentifier string
		expectedBody       []BodyStmt
	}{
		{"function (x) and (y)", []BodyStmt{{"x", "+", "y"}}},
		{"other function", []BodyStmt{
			{1.0, "*", 2.0},
		},
		},
	}

	l := lexer.New(input, nil, nil)
	p := New(l)

	program := p.ParseProgram()
	testProgram(t, program, 2)

	for i, tt := range tests {
		stmt := program.Statements[i]
		if !testDeclStatement(t, stmt, tt.expectedIdentifier, tt.expectedBody) {
			return
		}
	}

}

func TestReturnStatement(t *testing.T) {
	input := `= 5
	= 12.2`

	tests := []struct {
		expectedReturnValue interface{} // will change in the future
	}{
		{5.0},
		{12.2},
	}

	l := lexer.New(input, nil, nil)
	p := New(l)

	program := p.ParseProgram()
	testProgram(t, program, len(tests))

	for i, tt := range tests {
		stmt := program.Statements[i]
		if !testReturnStatement(t, stmt, tt.expectedReturnValue) {
			return
		}
	}

}

func TestIdentifierExpression(t *testing.T) {
	input := `foobar = 1
	foobar`

	tests := []struct {
		expectedValue string
	}{
		{"foobar"},
	}

	l := lexer.New(input, nil, nil)
	p := New(l)

	program := p.ParseProgram()
	testProgram(t, program, 2)

	for _, tt := range tests {
		stmt, ok := program.Statements[1].(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Statement is not type of expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
		}

		ident, ok := stmt.Expression.(*ast.Identifier)
		if !ok {
			t.Fatalf("Expression is not type of expected=%s, got=%T", "ast.Identifier", stmt.Expression)
		}

		testLiteralExpression(t, ident, tt.expectedValue)
	}
}

func TestNumberExpression(t *testing.T) {
	input := `5`

	tests := []struct {
		expectedValue   float64
		expectedLiteral string
	}{
		{5.0, "5"},
	}

	l := lexer.New(input, nil, nil)
	p := New(l)

	program := p.ParseProgram()
	testProgram(t, program, len(tests))

	for i, tt := range tests {
		stmt, ok := program.Statements[i].(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Statement is not type of expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
		}

		number, ok := stmt.Expression.(*ast.NumberLiteral)
		if !ok {
			t.Fatalf("Expression is not type of expected=%s, got=%T", "ast.Number", stmt.Expression)
		}

		testLiteralExpression(t, number, tt.expectedValue)

	}
}

func TestStringLiteralExpression(t *testing.T) {
	input := `"foobar"`

	tests := []struct {
		expectedValue   string
		expectedLiteral string
	}{
		{"foobar", "foobar"},
	}

	l := lexer.New(input, nil, nil)
	p := New(l)

	program := p.ParseProgram()
	testProgram(t, program, len(tests))

	for i, tt := range tests {
		stmt, ok := program.Statements[i].(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Statement is not type of expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
		}

		testStringLiteral(t, stmt.Expression, tt.expectedValue)

	}
}

func TestParsingPrefixExpression(t *testing.T) {
	prefixTests := []struct {
		input    string
		operator string
		value    interface{}
	}{
		{"!5", "!", 5.0},
		{"-15", "-", 15.0},
		{"* 15", "*", 15.0},
	}

	for _, tt := range prefixTests {
		l := lexer.New(tt.input, nil, nil)
		p := New(l)
		program := p.ParseProgram()

		testProgram(t, program, 1)

		stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Statement is not type of expected=%s, got=%T", "ast-ExpressionStatement", program.Statements[0])
		}

		exp, ok := stmt.Expression.(*ast.PrefixExpression)
		if !ok {
			t.Fatalf("stmt.Expression is not type of expected=%s, got=%t", "ast.PrefixExpression", stmt.Expression)
		}

		testPrefixExpression(t, exp, tt)
	}
}

func TestInfixExpressions(t *testing.T) {
	infixTests := []struct {
		input      string
		leftValue  interface{}
		operator   string
		rightValue interface{}
	}{
		{"5 + 5", 5.0, "+", 5.0},
		{"5 - 5", 5.0, "-", 5.0},
		{"5 * 5", 5.0, "*", 5.0},
		{"5 / 5", 5.0, "/", 5.0},
		{"5 > 5", 5.0, ">", 5.0},
		{"5 < 5", 5.0, "<", 5.0},
		{"5 == 5", 5.0, "==", 5.0},
		{"5 != 5", 5.0, "!=", 5.0},
		{"true == true", true, "==", true},
		{"false != false", false, "!=", false},
	}

	for _, tt := range infixTests {
		l := lexer.New(tt.input, nil, nil)
		p := New(l)
		program := p.ParseProgram()
		testProgram(t, program, 1)

		stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Statement is not type of expected=%s, got=%T", "*ast.ExpressionStatement", program.Statements[0])
		}

		exp, ok := stmt.Expression.(*ast.InfixExpression)
		if !ok {
			t.Fatalf("stmt.Expression is not type of expected=%s, got=%T", "*ast.InfixExpression", stmt.Expression)
		}

		testInfixExpression(t, exp, tt.leftValue, tt.operator, tt.rightValue)
	}
}

func TestOperatorPrecedenceParsing(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{
			"3 + 4 -5 * 5",
			"((3 + 4) - (5 * 5))",
		},
		{
			"5 > 4 == 3 < 4",
			"((5 > 4) == (3 < 4))",
		},
		{
			"5 < 4 != 3 > 4",
			"((5 < 4) != (3 > 4))",
		},
		{
			"3 + 4 * 5 == 3 * 1 + 4 * 5",
			"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
		},
		{
			"true",
			"true",
		},
		{
			"false",
			"false",
		},
		{
			"3 > 5 == false",
			"((3 > 5) == false)",
		},
		{
			"3 < 5 == true",
			"((3 < 5) == true)",
		},
		{
			"1 + (2 + 3) + 4",
			"((1 + (2 + 3)) + 4)",
		},
		{
			"(5 + 5) * 2",
			"((5 + 5) * 2)",
		},
		{
			"2 / (5 + 5)",
			"(2 / (5 + 5))",
		},
		{
			"-5 + 5",
			"((-5) + 5)",
		},
		{
			"!(true == true)",
			"(!(true == true))",
		},
		{
			"* 12 + 2",
			"* (12 + 2)",
		},
	}

	for _, tt := range tests {
		l := lexer.New(tt.input, nil, nil)
		p := New(l)
		program := p.ParseProgram()
		testProgram(t, program, 1)

		actual := program.String()
		if actual != tt.expected {
			t.Errorf("expected=%q, got=%q", tt.expected, actual)
		}
	}
}

func TestIfExpressions(t *testing.T) {
	input := `3 if 1 < 2`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()
	testProgram(t, program, 1)

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
	}

	exp, ok := stmt.Expression.(*ast.IfExpression)
	if !ok {
		t.Fatalf("Expression is not expected=%s, got=%T", "ast.IfExpression", stmt.Expression)
	}

	if !testInfixExpression(t, exp.Condition, 1, "<", 2) {
		return
	}

	if !testLiteralExpression(t, exp.Consequence, 3.0) {
		return
	}
}

func TestIfElseExpressions(t *testing.T) {
	input := `3 if 2 < 1 else 5`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()
	testProgram(t, program, 1)

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
	}

	exp, ok := stmt.Expression.(*ast.IfExpression)
	if !ok {
		t.Fatalf("Expression is not expected=%s, got=%T", "ast.IfExpression", stmt.Expression)
	}

	if !testInfixExpression(t, exp.Condition, 2, "<", 1) {
		return
	}

	if !testLiteralExpression(t, exp.Consequence, 3.0) {
		return
	}

	if !testLiteralExpression(t, exp.Alternative, 5.0) {
		return
	}
}

func TestCallExpressionParsing(t *testing.T) {
	input := `add 1 to 2

	# add (x) to (value y)
	x + value y`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()

	testProgram(t, program, 2)

	if !testDeclStatement(t, program.Statements[1], "add (x) to (value y)", []BodyStmt{{"x", "+", "value y"}}) {
		return
	}

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[1])
	}

	exp, ok := stmt.Expression.(*ast.CallExpression)
	if !ok {
		t.Fatalf("Expression is not expected=ast.CallExpression, got=%T", stmt.Expression)
	}

	if !testIdentifier(t, exp.Function, "add (x) to (value y)") {
		return
	}

	if len(exp.Arguments) != 2 {
		t.Fatalf("Call Arguments' incorrect length expected=%d, got=%d", 2, len(exp.Arguments))
	}

	testLiteralExpression(t, exp.Arguments[0], 1.0)
	testLiteralExpression(t, exp.Arguments[1], 2.0)

}

func TestAssignExpressions(t *testing.T) {
	input := `test = false`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()
	testProgram(t, program, 1)

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
	}

	exp, ok := stmt.Expression.(*ast.AssignExpression)
	if !ok {
		t.Fatalf("Expression is not expected=%s, got=%T", "ast.IfExpression", stmt.Expression)
	}

	if !testIdentifier(t, exp.Name, "test") {
		return
	}

	if !testLiteralExpression(t, exp.Value, false) {
		return
	}
}

func TestListExpressions(t *testing.T) {
	input := `* 12
	* 1 + 2
	* false`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()
	testProgram(t, program, 1)

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
	}

	exp, ok := stmt.Expression.(*ast.ListExpression)
	if !ok {
		t.Fatalf("Expression is not expected=%s, got=%T", "ast.ListExpression", stmt.Expression)
	}

	if !testLiteralExpression(t, exp.Elements[0], 12.0) {
		return
	}
	if !testInfixExpression(t, exp.Elements[1], 1, "+", 2) {
		return
	}
	if !testLiteralExpression(t, exp.Elements[2], false) {
		return
	}
}

func TestColonAssignExpressions(t *testing.T) {
	input := `test: 
	false`

	l := lexer.New(input, nil, nil)
	p := New(l)
	program := p.ParseProgram()
	testProgram(t, program, 1)

	stmt, ok := program.Statements[0].(*ast.ExpressionStatement)
	if !ok {
		t.Fatalf("Statement is not expected=%s, got=%T", "ast.ExpressionStatement", program.Statements[0])
	}

	exp, ok := stmt.Expression.(*ast.AssignExpression)
	if !ok {
		t.Fatalf("Expression is not expected=%s, got=%T", "ast.IfExpression", stmt.Expression)
	}

	if !testIdentifier(t, exp.Name, "test") {
		return
	}

	if !testLiteralExpression(t, exp.Value, false) {
		return
	}
}

func testProgram(t *testing.T, program *ast.Program, numOfTests int) {
	if program == nil {
		t.Fatalf("ParseProgram returned nil")
	}

	if numOfTests != len(program.Statements) {
		t.Fatalf("Incorrect number of Statements in Program\n expected=%d\n got=%d\n Statements: %+q\n", numOfTests, len(program.Statements), program.Statements)
	}
}

func testDeclStatement(
	t *testing.T,
	s ast.Statement,
	name string,
	body []BodyStmt) bool {
	if s.TokenLiteral() != "#" {
		t.Errorf("Statement has incorrect TokenLiteral expected=%s, got=%s", "#", s.TokenLiteral())
		return false
	}

	decl, ok := s.(*ast.Declaration)
	if !ok {
		t.Errorf("Statement is incorrect type, expected=%s, got=%T", "*ast.Declaration", s)
		return false
	}

	if !testIdentifier(t, decl.Name, name) {
		return false
	}

	if len(decl.Body.Statements) != len(body) {
		fmt.Printf("Body: %+v\n", decl.Body.Statements)
		t.Fatalf("Declaration Body is incorrect size expected=%d, got=%d", len(body), len(decl.Body.Statements))
	}

	for i, bodyStmt := range decl.Body.Statements {
		exp, ok := bodyStmt.(*ast.ExpressionStatement)
		if !ok {
			t.Fatalf("Body Statement not ExpressionStatement, got=%t", bodyStmt)
		}

		if !testInfixExpression(t, exp.Expression, body[i].left, body[i].operator, body[i].right) {
			return false
		}
	}
	return true
}

func testReturnStatement(t *testing.T, s ast.Statement, returnValue interface{}) bool {
	if s.TokenLiteral() != "=" {
		t.Errorf("Statement has incorrect TokenLiteral expected=%s, got=%s", "=", s.TokenLiteral())
		return false
	}

	returnStmt, ok := s.(*ast.ReturnStatement)
	if !ok {
		t.Errorf("Statement is incorrect type, expected=%s, got=%T", "*ast.ReturnStatement", s)
		return false
	}

	if returnStmt.TokenLiteral() != "=" {
		t.Errorf("ReturnStatement TokenLiteral is not expected=%s, got=%s", "=", returnStmt.TokenLiteral())
		return false
	}

	return testLiteralExpression(t, returnStmt.ReturnValue, returnValue)
}

func testLiteralExpression(t *testing.T, exp ast.Expression, expected interface{}) bool {
	switch v := expected.(type) {
	case float64:
		return testNumberLiteral(t, exp, float64(v))
	case int:
		return testNumberLiteral(t, exp, float64(v))
	case string:
		return testIdentifier(t, exp, v)
	case bool:
		return testBoolean(t, exp, v)
	}
	t.Errorf("Type of exp is not handled, got=%T", exp)
	return false
}

func testIdentifier(t *testing.T, exp ast.Expression, value string) bool {
	ident, ok := exp.(*ast.Identifier)
	if !ok {
		t.Errorf("exp is not *ast.Identifier, got:%T", exp)
		return false
	}

	if ident.Value != value {
		t.Errorf("Ident value is not %s, got=%s", value, ident.Value)
		return false
	}

	if ident.TokenLiteral() != value {
		t.Errorf("ident.TokenLiteral is not %s, got=%s", value, ident.TokenLiteral())
		return false
	}

	return true
}

func testNumberLiteral(t *testing.T, nl ast.Expression, value float64) bool {
	number, ok := nl.(*ast.NumberLiteral)
	if !ok {
		t.Fatalf("nl is not NumberLiteral, got=%t", nl)
		return false
	}

	if number.Value != value {
		t.Fatalf("number value is not %f, got=%f", value, number.Value)
		return false
	}

	if number.TokenLiteral() != strconv.FormatFloat(value, 'f', -1, 64) {
		t.Fatalf("number.TokenLiteral did not match the expected %s, got=%s", strconv.FormatFloat(value, 'f', -1, 64), number.TokenLiteral())
		return false
	}
	return true
}

func testStringLiteral(t *testing.T, nl ast.Expression, value string) bool {
	str, ok := nl.(*ast.StringLiteral)
	if !ok {
		t.Fatalf("nl is not StringLiteral, got=%T", nl)
		return false
	}

	if str.Value != value {
		t.Fatalf("number value is not %s, got=%s", value, str.Value)
		return false
	}

	if str.TokenLiteral() != value {
		t.Fatalf("number.TokenLiteral did not match the expected %s, got=%s", value, str.TokenLiteral())
		return false
	}
	return true
}

func testBoolean(t *testing.T, exp ast.Expression, test bool) bool {
	boolean, ok := exp.(*ast.Boolean)
	if !ok {
		t.Fatalf("exp is not Boolean, got=%t", exp)
		return false
	}

	if boolean.Value != test {
		t.Errorf("Boolean is not expected=%t, got=%t", test, boolean.Value)
		return false
	}
	return true
}

func testPrefixExpression(t *testing.T, exp *ast.PrefixExpression, test struct {
	input    string
	operator string
	value    interface{}
}) bool {
	if exp.Operator != test.operator {
		t.Errorf("exp.Operator is not expected=%s, got=%s", test.operator, exp.Operator)
		return false
	}

	return testLiteralExpression(t, exp.Right, test.value)
}

func testInfixExpression(t *testing.T, exp ast.Expression,
	leftValue interface{},
	operator string,
	rightValue interface{}) bool {
	infix, ok := exp.(*ast.InfixExpression)
	if !ok {
		t.Fatalf("Expression is not ast.InfixExpression, got=%T", exp)
	}

	if !testLiteralExpression(t, infix.Left, leftValue) {
		return false
	}

	if infix.Operator != operator {
		t.Errorf("exp.Operator is not expected=%s, got=%s", operator, infix.Operator)
		return false
	}

	if !testLiteralExpression(t, infix.Right, rightValue) {
		return false
	}
	return true
}
