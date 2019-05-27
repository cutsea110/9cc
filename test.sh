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

try 0 'int main () { return 0; }'
try 42 'int main () { return 42; }'

try 21 'int main () { return 5+20-4; }'
try 41 'int main () {  return 12 + 34 - 5 ; }'

try 47 'int main () { return 5+6*7; }'
try 15 'int main () { return 5*(9-6); }'
try 4 'int main () { return (3+5)/2; }'

try 6 'int main () { foo = 1; bar = 2 + 3; return foo + bar; }'
try 4 'int main () { foo = 2; return foo * foo; }'
try 16 'int main () { foo = 2; bar = foo * foo; return bar * bar; }'
try 14 'int main () { foo = 2; bar = foo * foo; return (bar + foo) + (bar * foo); }'
try 35 'int main () { foo=2; bar = foo + 3; foo=7; return foo*bar; }'

try 255 'int main () { return -1; }' # -1 ==> 255(shell)
try 42 'int main () { return +42; }'
try 12 'int main () { foo=-3;bar=foo*-5;return foo+bar; }'

try 1 'int main () { return 42 == 42; }'
try 0 'int main () { return 42 != 42; }'
try 1 'int main () { return 3+4 <= 7; }'
try 1 'int main () { return (3+4) <= (3*4); }'
try 0 'int main () { return 4 <= 3; }'
try 1 'int main () { return 4 <= 4; }'
try 1 'int main () { return 4 <= 5; }'
try 0 'int main () { return 4 < 3; }'
try 0 'int main () { return 4 < 4; }'
try 1 'int main () { return 4 < 5; }'
try 1 'int main () { return 4 >= 3; }'
try 1 'int main () { return 4 >= 4; }'
try 0 'int main () { return 4 >= 5; }'
try 1 'int main () { return 4 > 3; }'
try 0 'int main () { return 4 > 4; }'
try 0 'int main () { return 4 > 5; }'
try 0 'int main () { return 1 + 3 > 2 * 2; }'
try 1 'int main () { return 1 + 3 >= 2 * 2; }'

try 1 'int main () { a = 1; if (42 == 41) a = a + 41; return a; }'
try 42 'int main () { a = 1; if (42 == 42) a = a + 41; return a; }'
try 2 'int main () { a = 1; if (42 == 42) a = a + 1; else a = a + 2; return a; }'
try 3 'int main () { a = 1; if (42 == 41) a = a + 1; else a = a + 2; return a; }'

try 128 'int main () { i = 1; while (i <= 100) i = i * 2; return i; }'

try 10 'int main () { for (a=0;a<10;a=a+2)a;return a; }'
try 45 'int main () { total = 0; for (i = 0; i < 10; i = i+1) total = total + i; return total; }'
try 10 'int main () { total = 0; for (; total < 10;) total = total + 1; return total; }'
try 10 'int main () { for (total = 0; total < 10;) total = total + 1; return total; }'

try 1 'int main () { {} return 1; }'
try 3 'int main () { if (42 == 42) { return 3;} }'
try 3 'int main () { if (42 == 42) {return 3;} else {return 5;} }'
try 5 'int main () { if (42 == 41) {return 3;} else {return 5;} }'
try 5 'int main () { a = 2; b = 3; if (42 == 42) {return a + b;} else {return a * b;} }'
try 6 'int main () { a = 2; b = 3; if (42 == 41) {return a + b;} else {return a * b;} }'
try 55 'int main () { a = 1; b = 1; n = 0 ; while (n < 8) { tmp = a; a = b; b = tmp + a; n = n + 1;} return b; }' # fib
try 55 'int main () { total = 0; for (i = 0; i <= 10; i = i+1) { total = total + i;} return total; }' # sum

try 7 'int main () { return fun0(); }'
try 21 'int main () { a = fun0() + fun0(); return a + fun0(); }'
try 8 'int main () { return fun1(2); }'
try 5 'int main () { return fun2(2, 3); }'
try 25 'int main () { return fun3(7, 3, 4); }'
try 23 'int main () { return fun4(3, 7, 5, 2); }'
try 39 'int main () { return fun5(11, 3, 7, 5, 2); }'
try 27 'int main () { return fun6(11, 3, 7, 5, 2, 13); }'
try 4 'int main () { return twice(2); }'
try 16 'int main () { return twice(twice(2)); }'
try 255 'int main () { return twice(twice(twice(2))) - 1; }'
try 12 'int main () { x=0;for(i=0; i<3; i=i+1){ x = x + twice(2); } return x; }'

try 5 'int main () { return plus(2,3); } int plus(x,y) { return x+y; }'
try 1 'int main () { return fib(1); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 2 'int main () { return fib(2); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 3 'int main () { return fib(3); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 5 'int main () { return fib(4); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 8 'int main () { return fib(5); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 13 'int main () { return fib(6); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 21 'int main () { return fib(7); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 34 'int main () { return fib(8); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 55 'int main () { return fib(9); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 89 'int main () { return fib(10); } int fib (n){ if (n < 2) return 1; else return fib(n-1) + fib(n-2); }'
try 120 'int main () {return fact(5);} int fact(n) { if (n <= 1) { return 1; } else { return n * fact (n-1); }}'

echo OK
