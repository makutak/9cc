#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
#include <stdio.h>
int print3() { printf("3\n"); return 3; }
int print5() { printf("5\n"); return 5; }
int print_args(int a) { printf("a: %d\n", a); return a;}
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}
int sub6(int a, int b, int c, int d, int e, int f) {
  return a - b - c - d - e - f;
}
int sum(int i, int j) {
  int sum = 0;
  for (i; i <= j; i++) {
    sum = i + sum;
  }
  return sum;
}
EOF

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -static -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual"
	exit 1
    fi
}

assert 0 "main() { return 0; }"
assert 42 "main() { return 42;}"
assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return 12 + 34 - 5; }"
assert 47 'main() { return 5+6*7; }'
assert 15 'main() { return 5*(9-6); }'
assert 4 'main() { return (3+5)/2; }'
assert 10 'main() { return -10 + 20; }'
assert 10 'main() { return - -10; }'
assert 10 'main() { return - - +10; }'

assert 0 'main() { return 0==1; }'
assert 1 'main() { return 42==42; }'
assert 1 'main() { return 0!=1; }'
assert 0 'main() { return 42!=42; }'

assert 1 'main() { return 0<1; }'
assert 0 'main() { return 1<1; }'
assert 0 'main() { return 2<1; }'
assert 1 'main() { return 0<=1; }'
assert 1 'main() { return 1<=1; }'
assert 0 'main() { return 2<=1; }'

assert 1 'main() { return 1>0; }'
assert 0 'main() { return 1>1; }'
assert 0 'main() { return 1>2; }'
assert 1 'main() { return 1>=0; }'
assert 1 'main() { return 1>=1; }'
assert 0 'main() { return 1>=2; }'

assert 8 'main() { a=3 ;b=5; return a+b; }'
assert 14 'main() { a=3; b=5*6-8; return a+b/2; }'
assert 3 'main() { foo=1; bar=foo+2; return bar; }'

assert 3 'main() { foo = 1; bar = 2; return foo + bar; }'
assert 14 'main() { a = 3;b = 5 * 6 - 8;return a + b / 2; }'

assert 1 'main() { if (1) return 1; else return 0; }'
assert 0 'main() { if (1 == 0) return 1; else return 0; }'
assert 10 'main() { i=0; while(i<10) i=i+1; return i; }'
assert 11 'main() { i=0; while(i<=10) i=i+1; return i; }'

assert 55 'main() { i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'main() { for (;;) return 3; return 5; }'

assert 7 'main() { if (1) { foo = 7; return foo;} else {bar = 0; return bar;} }'
assert 0 'main() { if (0) { foo = 7; return foo;} else {bar = 0; return bar;} }'
assert 7 'main() { while(1) {foo = 7; return foo;} return 0; }'
assert 7 'main() { for(;;) {foo = 7; return foo;} return 0; }'
assert 3 'main() { return print3(); }'
assert 5 'main() { return print5(); }'


assert 10 'main() { return print_args(10); }'
assert 2 'main() { return add(1, 1); }'
assert 0 'main() { return sub(1, 1); }'
assert 21 'main() { return add6(1, 2, 3, 4, 5, 6); }'
assert 0 'main() { return sub6(15, 5, 4, 3, 2, 1); }'
assert 55 'main() { return sum(1, 10); }'

# assert 32 'main() { return ret32(); } ret32() { return 32; }'
# assert 7 'main() { return add2(3,4); } add2(x,y) { return x+y; }'
# assert 1 'main() { return sub2(4,3); } sub2(x,y) { return x-y; }'
# assert 55 'main() { return fib(9); } fib(x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

echo OK
