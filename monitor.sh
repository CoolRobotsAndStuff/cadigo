inotifywait -m . -e modify --include main\.c | while read events; do ./compile-run.sh; echo Reloading!; done
