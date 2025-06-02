int assert(int expected, int actual, char *code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
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

  printf("OK!\n");
  return 0;
}
