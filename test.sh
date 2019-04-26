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

echo OK
