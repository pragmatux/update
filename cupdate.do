: ${CC:=gcc}
redo-ifchange $2.o libupdate.so.1
$CC $2.o -o $3 -L. -lupdate
