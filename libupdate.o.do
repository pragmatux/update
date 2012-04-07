: ${CXX:=g++}
base=${2%.*}
$CXX --std=c++0x -Wall -MD -MF $base.d -c -fPIC -o $3 $base.cpp
read DEPS <$base.d
redo-ifchange $base.cpp ${DEPS#*:}
