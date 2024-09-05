inotifywait ./examples/ ./ -m -e modify --include .*\.\(c\|h\)\$ |
while read events
do
    ./compile-run.sh $1
    echo Reloading!
done
