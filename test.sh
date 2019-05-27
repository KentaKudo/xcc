#!/bin/sh

try () {
  expected="$1"
  input="$2"

  ./xcc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, got $actual"
    exit 1
  fi
}

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "-10+15;"
try 0 "-15 == 23;"
try 1 "23 != 3;"
try 1 "1 < 4;"
try 0 "4 <= -1;"
try 0 "4 > 9;"
try 1 "10 >= 8;"
try 33 "42; 33;"
try 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
try 5 "return 5; return 8;"
try 6 "foo = 1; bar = 2 + 3; return foo + bar;"
try 6 "foo1 = 1; foo2 = 2 + 3; return foo1 + foo2;"
try 42 "a = 1; if (a == 1) return 42; else return 33;"
try 33 "a = 0; if (a == 1) return 42; else return 33;"

echo OK
