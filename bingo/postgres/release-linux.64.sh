#!/bin/sh

make CONF=Release64
./bingo-release.sh $1 dist/x64/bingo_postgres.so
