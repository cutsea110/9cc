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

try 0 'main () { return 0; }'
try 42 'main () { return 42; }'

try 21 'main () { return 5+20-4; }'
try 41 'main () {  return 12 + 34 - 5 ; }'

try 47 'main () { return 5+6*7; }'
try 15 'main () { return 5*(9-6); }'
try 4 'main () { return (3+5)/2; }'

try 6 'main () { foo = 1; bar = 2 + 3; return foo + bar; }'
try 4 'main () { foo = 2; return foo * foo; }'
try 16 'main () { foo = 2; bar = foo * foo; return bar * bar; }'
try 14 'main () { foo = 2; bar = foo * foo; return (bar + foo) + (bar * foo); }'
try 35 'main () { foo=2; bar = foo + 3; foo=7; return foo*bar; }'

try 255 'main () { return -1; }' # -1 ==> 255(shell)
try 42 'main () { return +42; }'
try 12 'main () { foo=-3;bar=foo*-5;return foo+bar; }'

try 1 'main () { return 42 == 42; }'
try 0 'main () { return 42 != 42; }'
try 1 'main () { return 3+4 <= 7; }'
try 1 'main () { return (3+4) <= (3*4); }'
try 0 'main () { return 4 <= 3; }'
try 1 'main () { return 4 <= 4; }'
try 1 'main () { return 4 <= 5; }'
try 0 'main () { return 4 < 3; }'
try 0 'main () { return 4 < 4; }'
try 1 'main () { return 4 < 5; }'
try 1 'main () { return 4 >= 3; }'
try 1 'main () { return 4 >= 4; }'
try 0 'main () { return 4 >= 5; }'
try 1 'main () { return 4 > 3; }'
try 0 'main () { return 4 > 4; }'
try 0 'main () { return 4 > 5; }'
try 0 'main () { return 1 + 3 > 2 * 2; }'
try 1 'main () { return 1 + 3 >= 2 * 2; }'

try 1 'main () { a = 1; if (42 == 41) a = a + 41; return a; }'
try 42 'main () { a = 1; if (42 == 42) a = a + 41; return a; }'
try 2 'main () { a = 1; if (42 == 42) a = a + 1; else a = a + 2; return a; }'
try 3 'main () { a = 1; if (42 == 41) a = a + 1; else a = a + 2; return a; }'

try 128 'main () { i = 1; while (i <= 100) i = i * 2; return i; }'

try 10 'main () { for (a=0;a<10;a=a+2)a;return a; }'
try 45 'main () { total = 0; for (i = 0; i < 10; i = i+1) total = total + i; return total; }'
try 10 'main () { total = 0; for (; total < 10;) total = total + 1; return total; }'
try 10 'main () { for (total = 0; total < 10;) total = total + 1; return total; }'

try 1 'main () { {} return 1; }'
try 3 'main () { if (42 == 42) { return 3;} }'
try 3 'main () { if (42 == 42) {return 3;} else {return 5;} }'
try 5 'main () { if (42 == 41) {return 3;} else {return 5;} }'
try 5 'main () { a = 2; b = 3; if (42 == 42) {return a + b;} else {return a * b;} }'
try 6 'main () { a = 2; b = 3; if (42 == 41) {return a + b;} else {return a * b;} }'
try 55 'main () { a = 1; b = 1; n = 0 ; while (n < 8) { tmp = a; a = b; b = tmp + a; n = n + 1;} return b; }' # fib
try 55 'main () { total = 0; for (i = 0; i <= 10; i = i+1) { total = total + i;} return total; }' # sum

try 7 'main () { return fun0(); }'
try 21 'main () { a = fun0() + fun0(); return a + fun0(); }'
try 8 'main () { return fun1(2); }'
try 5 'main () { return fun2(2, 3); }'
try 25 'main () { return fun3(7, 3, 4); }'
try 23 'main () { return fun4(3, 7, 5, 2); }'
try 39 'main () { return fun5(11, 3, 7, 5, 2); }'
try 27 'main () { return fun6(11, 3, 7, 5, 2, 13); }'
try 4 'main () { return twice(2); }'
try 16 'main () { return twice(twice(2)); }'
try 255 'main () { return twice(twice(twice(2))) - 1; }'
try 12 'main () { x=0;for(i=0; i<3; i=i+1){ x = x + twice(2); } return x; }'

try 5 'main () { return plus(2,3); } plus(x,y) { return x+y; }'
try 1 'main () { return fib(1); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 2 'main () { return fib(2); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 3 'main () { return fib(3); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 5 'main () { return fib(4); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 8 'main () { return fib(5); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 13 'main () { return fib(6); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 21 'main () { return fib(7); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 34 'main () { return fib(8); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 55 'main () { return fib(9); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 89 'main () { return fib(10); } fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 120 'main () {return fact(5);} fact(n) { if (n <= 1) { return 1; } else { return n * fact (n-1); }}'

echo OK
