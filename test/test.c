int assert(int expected, int actual, char *code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
}

/* void alloc4(int **p, int a, int b, int c, int d) { */
/*   *p = malloc(sizeof(int) * 4); */
/*   (*p)[0] = a; */
/*   (*p)[1] = b; */
/*   (*p)[2] = c; */
/*   (*p)[3] = d; */
/* } */

int return_3() {
  return 3;
}

int return_5() {
  return 5;
}

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

int add6(int a, int b, int c, int d, int e, int f) {
  return a + b + c + d + e + f;
}

int sub6(int a, int b, int c, int d, int e, int f) {
  return a - b - c - d - e - f;
}

int sum(int i, int j) {
  int sum = 0;

  // TODO: i++,i-- の対応
  for (i; i <= j; i = i + 1) {
    sum = i + sum;
  }
  return sum;
}

int ret32() {
  return 32;
}

int add2(int x, int y) {
  return x + y;
}

int sub2(int x, int y) {
  return x - y;
}

int fib(int x) {
  if (x <= 1)
    return 1;
  return fib(x - 1) + fib(x - 2);
}

int foo(int *x, int y) {
  return *x + y;
}

int main() {
  assert(0, 0, "0");
  assert(42, 42, "42");
  assert(21, 5 + 20 - 4, "5+20-4");
  assert(41, 12 + 34 - 5, "12 + 34 - 5");
  assert(47, 5 + 6 * 7, "5+6*7");
  assert(15, 5 * (9 - 6), "5*(9-6)");
  assert(4, (3 + 5) / 2, "(3+5)/2");
  assert(10, -10 + 20, "-10 + 20");
  assert(10, - -10, "- -10");
  assert(10, - -+10, "- - +10");

  assert(0, 0 == 1, "0==1");
  assert(1, 42 == 42, "42==42");
  assert(1, 0 != 1, "0!=1");
  assert(0, 42 != 42, "42!=42");

  assert(1, 0 < 1, "0<1");
  assert(0, 1 < 1, "1<1");
  assert(0, 2 < 1, "2<1");
  assert(1, 0 <= 1, "0<=1");
  assert(1, 1 <= 1, "1<=1");
  assert(0, 2 <= 1, "2<=1");

  assert(1, 1 > 0, "1>0");
  assert(0, 1 > 1, "1>1");
  assert(0, 1 > 2, "1>2");
  assert(1, 1 >= 0, "1>=0");
  assert(1, 1 >= 1, "1>=1");
  assert(0, 1 >= 2, "1>=2");

  assert(8, ({
           int a;
           int b;
           a = 3;
           b = 5;
           a + b;
         }),
         "{ int a; int b; a=3 ;b=5; a+b; }");
  assert(14, ({
           int a;
           int b;
           a = 3;
           b = 5 * 6 - 8;
           a + b / 2;
         }),
         "int a; int b; a=3; b=5*6-8; a+b/2;");
  assert(3, ({
           int foo;
           int bar;
           foo = 1;
           bar = foo + 2;
           bar;
         }),
         "int foo; int bar; foo=1; bar=foo+2; bar;");
  assert(3, ({
           int foo;
           int bar;
           foo = 1;
           bar = 2;
           foo + bar;
         }),
         "int foo; int bar; foo = 1; bar = 2; foo + bar;");
  assert(14, ({
           int a;
           int b;
           a = 3;
           b = 5 * 6 - 8;
           a + b / 2;
         }),
         "int a; int b; a = 3;b = 5 * 6 - 8;a + b / 2;");

  assert(3, ({
           int x = 0;
           if (0)
             x = 2;
           else
             x = 3;
           x;
         }),
         "int x = 0; if (1) x = 2; else  x = 3; x;");

  assert(0, ({
           int x = 0;
           if (1 == 0)
             x = 1;
           else
             x = 0;
           x;
         }),
         "int x = 0; if (1 == 0) x = 1; else x = 0; x;");
  assert(10, ({
           int i = 0;
           while (i < 10)
             i = i + 1;
           i;
         }),
         "int i =0; while(i<10) i=i+1; i;");
  assert(11, ({
           int i = 0;
           while (i <= 10)
             i = i + 1;
           i;
         }),
         "int i =0; while(i<=10) i=i+1; i;");

  assert(55, ({
           int i = 0;
           int j = 0;
           for (i = 0; i <= 10; i = i + 1)
             j = i + j;
           j;
         }),
         "int i = 0; int j = 0; for (i=0; i<=10; i=i+1) j=i+j; j;");

  assert(3, return_3(), "return_3();");
  assert(5, return_5(), "return_5();");
  assert(2, add(1, 1), "add(1, 1)");
  assert(0, sub(1, 1), "sub(1, 1)");
  assert(21, add6(1, 2, 3, 4, 5, 6), "add6(1, 2, 3, 4, 5, 6)");
  assert(0, sub6(15, 5, 4, 3, 2, 1), "sub6(15, 5, 4, 3, 2, 1)");
  assert(55, sum(1, 10), "sum(1, 10)");
  assert(32, ret32(), "ret32()");
  assert(7, add2(3, 4), "add2(3,4)");
  assert(1, sub2(4, 3), "sub2(4,3)");
  assert(55, fib(9), "fib(9)");

  assert(3, ({
           int x;
           x = 3;
           *&x;
         }),
         "int x; x = 3; *&x;");
  assert(3, ({
           int x = 3;
           int *p = &x;
           *p;
         }),
         "int x=3; int *p=&x; *p;");

  assert(3, ({
           int x = 3;
           *&x;
         }),
         "int x=3; *&x;");

  assert(3, ({
           int x = 3;
           int *y = &x;
           int **z = &y;
           **z;
         }),
         "int x=3; int *y=&x; int **z=&y; **z;");
  assert(5, ({
           int x = 3;
           int y = 5;
           *(&x + 1);
         }),
         "int x=3; int y=5; *(&x+1);");
  assert(5, ({
           int x = 3;
           int y = 5;
           *(1 + &x);
         }),
         "int x=3; int y=5; *(1+&x);");
  assert(3, ({
           int x = 3;
           int y = 5;
           *(&y - 1);
         }),
         "int x=3; int y=5;  *(&y-1);");
  assert(5, ({
           int x = 3;
           int y = 5;
           int *z = &x;
           *(z + 1);
         }),
         "int x=3; int y=5; int *z=&x; *(z+1);");
  assert(3, ({
           int x = 3;
           int y = 5;
           int *z = &y;
           *(z - 1);
         }),
         "int x=3; int y=5; int *z=&y; *(z-1);");
  assert(5, ({
           int x = 3;
           int *y = &x;
           *y = 5;
           x;
         }),
         "int x=3; int *y=&x; *y=5; x;");
  assert(7, ({
           int x = 3;
           int y = 5;
           *(&x + 1) = 7;
           y;
         }),
         "int x=3; int y=5; *(&x+1)=7; y;");
  assert(7, ({
           int x = 3;
           int y = 5;
           *(&y - 1) = 7;
           x;
         }),
         "int x=3; int y=5; *(&y-1)=7; x;");
  assert(8, ({
           int x = 3;
           int y = 5;
           foo(&x, y);
         }),
         "int x=3; int y=5; foo(&x, y);");

  assert(4, ({
           int x;
           sizeof(x);
         }),
         "int x; sizeof(x);");
  assert(8, ({
           int *y;
           sizeof(y);
         }),
         "int *y; sizeof(y);");
  assert(4, ({
           int x;
           sizeof(x + 3);
         }),
         "int x; sizeof(x + 3);");
  assert(8, ({
           int *y;
           sizeof(y + 3);
         }),
         "int *y; sizeof(y + 3);");
  assert(4, ({
           int *y;
           sizeof(*y);
         }),
         "int *y; sizeof(*y);");
  assert(4, ({ sizeof(1); }), "sizeof(1);");
  // 本来はsize_tなので8になる
  assert(4, ({ sizeof(sizeof(1)); }), "sizeof(sizeof(1));");

  assert(4, ({
           int x;
           sizeof x;
         }),
         "int x; sizeof x;");
  assert(8, ({
           int *y;
           sizeof y;
         }),
         "int *y; sizeof y;");
  assert(7, ({
           int x;
           sizeof x + 3;
         }),
         "int x; sizeof x + 3;");
  assert(11, ({
           int *y;
           sizeof y + 3;
         }),
         "int *y; sizeof y + 3;");
  assert(4, ({
           int *y;
           sizeof *y;
         }),
         "int *y; sizeof *y;");
  assert(4, ({ sizeof 1; }), "sizeof 1;");
  // 本来はsize_tなので8になる
  assert(4, ({ sizeof sizeof 1; }), "sizeof sizeof 1;");

  assert(1, ({
           char x = 1;
           x;
         }),
         "char x=1; x;");
  assert(1, ({
           char x = 1;
           char y = 2;
           x;
         }),
         "char x=1; char y=2; x;");
  assert(2, ({
           char x = 1;
           char y = 2;
           y;
         }),
         "char x=1; char y=2; y;");

  printf("OK!\n");
  return 0;
}
