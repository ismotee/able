package evaluator

import (
	"able/ast"
	"able/object"
	"fmt"
)

var (
	NULL  = &object.Null{}
	TRUE  = &object.Boolean{Value: true}
	FALSE = &object.Boolean{Value: false}
)

func SortAst(program *ast.Program) {
	declarations := []ast.Statement{}
	other := []ast.Statement{}
	for _, stmt := range program.Statements {
		decl, ok := stmt.(*ast.Declaration)
		if ok {
			declarations = append(declarations, decl)
		} else {
			other = append(other, stmt)
		}
	}
	program.Statements = append(declarations, other...)
}

func Eval(node ast.Node, env *object.Environment) object.Object {
	switch node := node.(type) {
	case *ast.Program:
		SortAst(node)
		return evalProgram(node.Statements, env)
	case *ast.ExpressionStatement:
		return Eval(node.Expression, env)
	case *ast.PrefixExpression:
		right := Eval(node.Right, env)
		return evalPrefixExpression(node.Operator, right)
	case *ast.InfixExpression:
		left := Eval(node.Left, env)
		right := Eval(node.Right, env)
		return evalInfixExpression(node.Operator, left, right)
	case *ast.IfExpression:
		return evalIfExpression(node, env)
	case *ast.ReturnStatement:
		val := Eval(node.ReturnValue, env)
		return &object.ReturnValue{Value: val}
	case *ast.AssignExpression:
		val := Eval(node.Value, env)
		// check for errors
		env.Set(node.Name.Value, val)
	case *ast.Declaration:
		val := &object.Function{Parameters: node.Parameters, Body: node.Body, Env: env}
		env.Set(node.Name.Value, val)
	case *ast.CallExpression:
		function := Eval(node.Function, env)
		// check function for errors
		args := evalExpressions(node.Arguments, env)
		// check args for errors
		return applyFunction(function, args)
	case *ast.BlockStatement:
		return evalBlockStatement(node, env)

	// expressions
	case *ast.NumberLiteral:
		return &object.Number{Value: node.Value}
	case *ast.Boolean:
		return nativeBoolToBooleanObject(node.Value)
	case *ast.StringLiteral:
		return &object.String{Value: node.Value}
	case *ast.Identifier:
		return evalIdentifier(node, env)
	}

	return nil
}

func evalProgram(stmts []ast.Statement, env *object.Environment) object.Object {
	var result object.Object
	for _, stmt := range stmts {
		result = Eval(stmt, env)
		if result != nil && result.Type() != object.NULL_OBJ {
			fmt.Println(result.Inspect())
		}

		if returnValue, ok := result.(*object.ReturnValue); ok {
			return returnValue.Value
		}
	}
	return result
}

func evalPrefixExpression(operator string, right object.Object) object.Object {
	switch operator {
	case "!":
		return evalBangOperatorExpression(right)
	case "-":
		return evalMinusOperatorExpression(right)
	default:
		return NULL
	}
}

func evalBangOperatorExpression(right object.Object) object.Object {
	switch right {
	case TRUE:
		return FALSE
	case FALSE:
		return TRUE
	case NULL:
		return TRUE
	default:
		return FALSE
	}
}

func nativeBoolToBooleanObject(input bool) object.Object {
	if input {
		return TRUE
	}
	return FALSE
}

func evalMinusOperatorExpression(right object.Object) object.Object {
	if right.Type() != object.NUMBER_OBJ {
		return NULL
	}

	value := right.(*object.Number).Value
	return &object.Number{Value: -value}
}

func evalInfixExpression(operator string, left, right object.Object) object.Object {
	switch {
	case left.Type() == object.NUMBER_OBJ && right.Type() == object.NUMBER_OBJ:
		return evalNumberInfixExpression(operator, left, right)
	case left.Type() == object.STRING_OBJ && right.Type() == object.STRING_OBJ:
		return evalStringInfixExpression(operator, left, right)
	case operator == "==":
		return nativeBoolToBooleanObject(left == right)
	case operator == "!=":
		return nativeBoolToBooleanObject(left != right)
	default:
		return NULL
	}
}

func evalNumberInfixExpression(operator string, left, right object.Object) object.Object {
	leftVal := left.(*object.Number).Value
	rightVal := right.(*object.Number).Value

	switch operator {
	case "+":
		return &object.Number{Value: leftVal + rightVal}
	case "-":
		return &object.Number{Value: leftVal - rightVal}
	case "*":
		return &object.Number{Value: leftVal * rightVal}
	case "/":
		return &object.Number{Value: leftVal / rightVal}
	case "<":
		return nativeBoolToBooleanObject(leftVal < rightVal)
	case ">":
		return nativeBoolToBooleanObject(leftVal > rightVal)
	case "==":
		return nativeBoolToBooleanObject(leftVal == rightVal)
	case "!=":
		return nativeBoolToBooleanObject(leftVal != rightVal)
	default:
		return NULL
	}
}

func evalStringInfixExpression(op string, left, right object.Object) object.Object {
	if op != "+" {
		// handle error or make some wonky find-remove operations
		return NULL
	}
	leftVal := left.(*object.String).Value
	rightVal := right.(*object.String).Value
	return &object.String{Value: leftVal + rightVal}
}

func evalIfExpression(ie *ast.IfExpression, env *object.Environment) object.Object {
	condition := Eval(ie.Condition, env)

	if isTruthy(condition) {
		return Eval(ie.Consequence, env)
	} else if ie.Alternative != nil {
		return Eval(ie.Alternative, env)
	}
	return NULL
}

func evalIdentifier(node *ast.Identifier, env *object.Environment) object.Object {
	val, ok := env.Get(node.Value)
	if !ok {
		fmt.Printf("identifier '%s' not found\n", node.Value)
		// error identifier not found
		return nil
	}
	return val
}

func isTruthy(obj object.Object) bool {
	switch obj {
	case NULL:
		return false
	case TRUE:
		return true
	case FALSE:
		return false
	default:
		return true
	}
}

func evalExpressions(exps []ast.Expression, env *object.Environment) []object.Object {
	var result []object.Object

	for _, e := range exps {
		evaluated := Eval(e, env)
		// check and return error
		result = append(result, evaluated)
	}

	return result
}

func applyFunction(fn object.Object, args []object.Object) object.Object {
	function := fn.(*object.Function)
	// check for error

	extendedEnv := extendFunctionEnv(function, args)
	evaluated := Eval(function.Body, extendedEnv)
	return unwrapReturnValue(evaluated)
}

func extendFunctionEnv(fn *object.Function, args []object.Object) *object.Environment {
	env := object.NewEnclosedEnvironment(fn.Env)

	for paramIdx, param := range fn.Parameters {
		env.Set(param.Value, args[paramIdx])
	}
	return env
}

func unwrapReturnValue(obj object.Object) object.Object {
	if returnValue, ok := obj.(*object.ReturnValue); ok {
		return returnValue.Value
	}

	return obj
}

func evalBlockStatement(block *ast.BlockStatement, env *object.Environment) object.Object {
	var result object.Object

	for _, statement := range block.Statements {
		result = Eval(statement, env)

		if result != nil && result.Type() == object.RETURN_VALUE_OBJ {
			return result
		}
	}

	return result
}
