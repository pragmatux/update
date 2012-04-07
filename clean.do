exec >/dev/null 2>&1
redo-targets | xargs rm -f
rm -f *.d
rm -rf .redo
