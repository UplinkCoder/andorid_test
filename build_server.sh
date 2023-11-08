#/bin/sh
gcc lazyload.c -c -o lazyload.o
gcc server.c lazyload.o -lsqlite3 -ldl
