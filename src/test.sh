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

# assert 0 0
# assert 42 42
# assert 21 "5+20-4"
# assert 41 " 12 +34 - 5 "
# assert 47 '5+6*7'
# assert 15 '5*(9-6)'
# assert 4 '(3+5)/2'
# assert 10 '-10+20'
# assert 1 '1 == 1'
# assert 1 '2 > 1'
# assert 1 '10 >= 10'
# assert 1 '10 <= 10'
# assert 0 '11 < 10'

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
# assert 1 "1 < 2;"
# assert 0 "1 > 2;"

echo OK