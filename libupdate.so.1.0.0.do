: ${CC:=gcc}
redo-ifchange libupdate.o
$CC -shared -Wl,-soname,${2%.*.*} -o $3 libupdate.o -lstdc++
