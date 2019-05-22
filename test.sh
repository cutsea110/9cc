#!/bin/bash
try(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s func.o
    ./tmp
    actual="$?"

    echo -n "$input => "
    
    if [ "$actual" = "$expected" ]; then
	echo "$actual"
    else
	echo "ERROR!!"
	echo "  $expected expected, but got $actual"
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

try 255 'return -1;' # -1 ==> 255(shell)
try 42 'return +42;'
try 12 'foo=-3;bar=foo*-5;return foo+bar;'

try 1 'return 42 == 42;'
try 0 'return 42 != 42;'
try 1 'return 3+4 <= 7;'
try 1 'return (3+4) <= (3*4);'
try 0 'return 4 <= 3;'
try 1 'return 4 <= 4;'
try 1 'return 4 <= 5;'
try 0 'return 4 < 3;'
try 0 'return 4 < 4;'
try 1 'return 4 < 5;'
try 1 'return 4 >= 3;'
try 1 'return 4 >= 4;'
try 0 'return 4 >= 5;'
try 1 'return 4 > 3;'
try 0 'return 4 > 4;'
try 0 'return 4 > 5;'
try 0 'return 1 + 3 > 2 * 2;'
try 1 'return 1 + 3 >= 2 * 2;'

try 1 'a = 1; if (42 == 41) a = a + 41; return a;'
try 42 'a = 1; if (42 == 42) a = a + 41; return a;'
try 2 'a = 1; if (42 == 42) a = a + 1; else a = a + 2; return a;'
try 3 'a = 1; if (42 == 41) a = a + 1; else a = a + 2; return a;'

try 128 'i = 1; while (i <= 100) i = i * 2; return i;'

try 10 'for (a=0;a<10;a=a+2)a;return a;'
try 45 'total = 0; for (i = 0; i < 10; i = i+1) total = total + i; return total;'
try 10 'total = 0; for (; total < 10;) total = total + 1; return total;'
try 10 'for (total = 0; total < 10;) total = total + 1; return total;'

try 1 '{} return 1;'
try 3 'if (42 == 42) { return 3;}'
try 3 'if (42 == 42) {return 3;} else {return 5;}'
try 5 'if (42 == 41) {return 3;} else {return 5;}'
try 5 'a = 2; b = 3; if (42 == 42) {return a + b;} else {return a * b;}'
try 6 'a = 2; b = 3; if (42 == 41) {return a + b;} else {return a * b;}'
try 55 'a = 1; b = 1; n = 0 ; while (n < 8) { tmp = a; a = b; b = tmp + a; n = n + 1;} return b;' # fib
try 55 'total = 0; for (i = 0; i <= 10; i = i+1) { total = total + i;} return total;' # sum

try 7 'return fun0();'
try 21 'a = fun0() + fun0(); return a + fun0();'
try 8 'return fun1(2);'
try 5 'return fun2(2, 3);'
try 25 'return fun3(7, 3, 4);'

echo OK
