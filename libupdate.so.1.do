real=$(ls $1.*.*.do)
real=${real%.do}
redo-ifchange $real
rm -f $1
ln -s $real $3
