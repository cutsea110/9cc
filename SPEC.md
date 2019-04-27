SPECIFICATION
=================


SYNTAX
----------

program: stmt program
program: e

stmt: "return" assign ";"
stmt: assign ";"

assign: add
assign: add "=" assign

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
