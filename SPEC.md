SPECIFICATION
=================


SYNTAX
----------

program: stmt program
program: e

stmt: "return" assign ";"
stmt: assign ";"

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
term: "(" assign ")"
