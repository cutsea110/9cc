#!/bin/bash
try(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$expected expected, but got $actual"
	exit 1
    fi
}

try 0 'return 0;'
try 42 'return 42;'

try 21 'return 5+20-4;'
try 41 ' return 12 + 34 - 5 ;'

try 47 'return 5+6*7;'
try 15 'return 5*(9-6);'
try 4 'return (3+5)/2;'

try 6 'foo = 1; bar = 2 + 3; return foo + bar;'
try 4 'foo = 2; return foo * foo;'
try 16 'foo = 2; bar = foo * foo; return bar * bar;'
try 14 'foo = 2; bar = foo * foo; return (bar + foo) + (bar * foo);'
try 35 'foo=2; bar = foo + 3; foo=7; return foo*bar;'

#try -1 'return -1;'
try 42 'return +42;'
try 12 'foo=-3;bar=foo*-5;return foo+bar;'

try 1 'return 42 == 42;'
try 0 'return 42 != 42;'
try 1 'return 3+4 <= 7;'
try 1 'return (3+4) <= (3*4);'
try 0 'return 4 <= 3;'
try 0 'return 4 < 4;'
try 0 'return 1 + 3 > 2 * 2;'
try 1 'return 1 + 3 >= 2 * 2;'

try 3 'if (42 == 42) return 3;'

echo OK
