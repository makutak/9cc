#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
#include <stdio.h>
#include <stdlib.h>

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


void alloc4(int **p, int a, int b, int c, int d) {
  *p = malloc(sizeof(long) * 4);
  ((long *)(*p))[0] = a;
  ((long *)(*p))[1] = b;
  ((long *)(*p))[2] = c;
  ((long *)(*p))[3] = d;
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

assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42;}"
assert 21 "int main() { return 5+20-4; }"
assert 41 "int main() { return 12 + 34 - 5; }"
assert 47 'int main() { return 5+6*7; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'
assert 10 'int main() { return -10 + 20; }'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 42==42; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 42!=42; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 8 'int main() { int a; int b; a=3 ;b=5; return a+b; }'
assert 14 'int main() { int a; int b; a=3; b=5*6-8; return a+b/2; }'
assert 3 'int main() { int foo; int bar; foo=1; bar=foo+2; return bar; }'

assert 3 'int main() { int foo; int bar; foo = 1; bar = 2; return foo + bar; }'
assert 14 'int main() { int a; int b; a = 3;b = 5 * 6 - 8;return a + b / 2; }'

assert 1 'int main() { if (1) return 1; else return 0; }'
assert 0 'int main() { if (1 == 0) return 1; else return 0; }'
assert 10 'int main() { int i; i=0; while(i<10) i=i+1; return i; }'
assert 11 'int main() { int i; i=0; while(i<=10) i=i+1; return i; }'

assert 55 'int main() { int i; int j; i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main() { for (;;) return 3; return 5; }'

assert 7 'int main() { if (1) { int foo; foo = 7; return foo;} else {int bar; bar = 0; return bar;} }'
assert 0 'int main() { int foo; int bar; if (0) { foo = 7; return foo;} else {bar = 0; return bar;} }'
assert 7 'int main() { int foo; int bar; while(1) {foo = 7; return foo;} return 0; }'
assert 7 'int main() { int foo; for(;;) {foo = 7; return foo;} return 0; }'
assert 3 'int main() { return print3(); }'
assert 5 'int main() { return print5(); }'


assert 10 'int main() { return print_args(10); }'
assert 2 'int main() { return add(1, 1); }'
assert 0 'int main() { return sub(1, 1); }'
assert 21 'int main() { return add6(1, 2, 3, 4, 5, 6); }'
assert 0 'int main() { return sub6(15, 5, 4, 3, 2, 1); }'
assert 55 'int main() { return sum(1, 10); }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x,int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x,int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

# assert 3 'int main() { int x; int y; x = 3; y = &x; return *y; }'
# assert 3 'int main() { int x; x = 3; return *&x; }'
# assert 3 'int main() { int x; int y; x = 3; y = &x; return *y; }'
# assert 3 'int main() { int x; int y; x = 3; y = 5; return *(&y-8); }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 5 'int main() { int x=3; int y=5; return *(1+&x); }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 5 'int main() { int x=3; int y=5; int *z=&x; return *(z+1); }'
assert 3 'int main() { int x=3; int y=5; int *z=&y; return *(z-1); }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-1)=7; return x; }'
assert 8 'int main() { int x=3; int y=5; return foo(&x, y); } int foo(int *x, int y) { return *x + y; }'

assert 8 'int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; *q; q = p + 3; return *q; }'

echo OK
