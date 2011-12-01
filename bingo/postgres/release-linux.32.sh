#!/bin/sh

make CONF=Release32
./bingo-release.sh $1 dist/x86/bingo_postgres.so
