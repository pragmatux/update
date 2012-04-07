: ${CC:=gcc}
base=${2%.*}
$CC -Wall -MD -MF $base.d -c -o $3 -I. $base.c
read DEPS <$base.d
redo-ifchange $base.c ${DEPS#*:}
