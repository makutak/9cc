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

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10 + 20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 8 'a=3;b=5;a+b;'
assert 14 'a=3;b=5*6-8;a+b/2;'
assert 3 'foo=1; bar=foo+2;'

assert 3 'foo = 1; bar = 2; return foo + bar;'
assert 14 'a = 3;b = 5 * 6 - 8;return a + b / 2;'

assert 1 'if (1) return 1; else return 0;'
assert 0 'if (1 == 0) return 1; else return 0;'
assert 10 'i=0; while(i<10) i=i+1; return i;'
assert 11 'i=0; while(i<=10) i=i+1; return i;'

assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert 3 'for (;;) return 3; return 5;'

assert 7 'if (1) { foo = 7; return foo;} else {bar = 0; return bar;}'
assert 0 'if (0) { foo = 7; return foo;} else {bar = 0; return bar;}'
assert 7 'while(1) {foo = 7; return foo;} return 0;'
assert 7 'for(;;) {foo = 7; return foo;} return 0;'
assert 3 'return print3();'
assert 5 'return print5();'
assert 10 'return print_args(10);'
assert 2 'return add(1, 1);'
assert 0 'return sub(1, 1);'
assert 21 'return add6(1, 2, 3, 4, 5, 6);'
assert 0 'return sub6(15, 5, 4, 3, 2, 1);'

echo OK
