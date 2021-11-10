#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}


assert 1 "var=1;var;"
assert 2 "a=1;a+1;"
# assert 2 "a=1;return a+1;"
# assert 3 "a=1;return 3;"
# assert 1 "a=1;return a == 1;"
# assert 1 "a=1;if ( a == 1 ) return a;"
# assert 2 "a=2;if ( a == 2 ) return a;"
# assert 0 "return 10 > 11;"
# assert 0 "return 10 == 11;"
# assert 1 "11 == 11;"
assert 10 "if (1 == 1) return 10;"
assert 10 "if (2 > 1) return 10;"
# assert 2 "if (2 < 1) return 10;"
# assert 1 "1 < 2;"
# assert 0 "1 > 2;"
assert 10 "if (1 < 0) return 5; else return 10;"
assert 10 "i = 0;  while (i < 10) i = i + 1; return i;"
assert 10 "i = 0;  for (a = 0; a < 10; a=a+1) i = i + 1; return i;"
assert 10 "i = 0;  a=0; for (; a < 10; a=a+1) i = i + 1; return i;"

echo OK