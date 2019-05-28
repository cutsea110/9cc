SPECIFICATION
=================


SYNTAX
----------

program: decl program
program: e

decl: tsig ident ";"
decl: tsig ident "(" (tsig expr)? ("," tsig expr)* ")" "{" stmt* "}"

tsig: "int" "*"*

stmt: tsig ident ";"
stmt: expr ";"
stmt: "{" stmt* "}"
stmt: "return" expr ";"
stmt: "if" "(" expr ")" stmt
stmt: "if" "(" expr ")" stmt "else" stmt
stmt: "while" "(" expr ")" stmt
stmt: "for" "(" expr? ";" expr? ";" expr? ")" stmt

expr: assign

assign: equality
assign: equality "=" assign

equality: relational
equality: equality "==" relational
equality: equality "!=" relational

relational: add
relational: relational "<"  add
relational: relational "<=" add
relational: relational ">"  add
relational: relational ">=" add

add: mul
add: add "+" mul
add: add "-" mul

mul: unary
mul: mul "*" unary
mul: mul "/" unary

unary: "+" term
unary: "-" term
unary: term

term: num
term: ident
term: ident "(" expr? ("," expr)* ")"
term: "(" expr ")"
