# Able parser processes

## Lexer

Makes initial Syntax Token array.

## Semantic Analyzer

Semantic analyzer modifies and interprets all word tokens. This is done in five steps:

- Links
- Declarations
- Assignments
- Identifiers
- Key phrases

Every step respectfully reduce word tokens into more accurate tokens until it is easy to form a AST (Abstract syntax tree).

### Links

Links are on the form of [alias][path] where path points somewhere in the project. Path can have filename and declaration identifier.
Markdaown automatically forms a reference from a header (Declaration in Able). To form a reference markdown will write header all lower case and
it replaces whitespaces with "-". Also Brackets and other special characters will be eliminated. This is a good thing for Able because Declaration
can have braces and markdown can still convert those headers to references.

File name and reference will be separated with "#". The same hash character is used in declarations. Links are easier to handle because path is wrapped
in brackets. After Parsing the Links it should be relatively easy to parse Declaration tokens.

Aliases should be gathered to namespaces.

### Declarations

Parsing declarations is pretty straight forward after links are parsed. Declarations start always with one or more Hash tokens followed by name of the declaration. Declaration ends with End Line or End Of File Token. Name cannot be empty.

### Assignments

There is two kind of assignments: One with the Colon Token and other with Equal Token. In both cases the challenge for parser is to have Assign Token first and identifier after that. After Identifier token there should be a With Token and expression that determines the value of the assigned identifier.

Verbosed tokens should something like:
`ASSIGN foo WITH bar`

### Namespace or Scopes

Now that there's identifiers and declarations, it is time to gather these into a namespace. It is used in the next phase where (mainly) Words will be interpreted into identifiers and calls.

### Identifiers and Calls

### Key phrases

Key phrases such as "join (x) with (y)" or "print" will be parsed the last. This means that calls for example can include the same words than phrases. However any declaration or assignment shouldn't be exactly the same as key phrase. This check of course must done during the declaration and assignment stages.

Key phrase detection needs some kind of pattern in which is then compared to syntax tokens. Parameters in Key phrases are unnamed for making the code little bit simpler.

### Operations

Last phse is to compose arithmetic and logical opartions. Effectively this means +, -, *, / operations and comparisons such as ==, <, > ,!=, =>, =<. The code should recognize the right count order between these operations. Functions should be calculated first.

## Semantic tree

There should not be any Word tokens left after Namepace analyzer. If there's one, it should throw unknown word error.
