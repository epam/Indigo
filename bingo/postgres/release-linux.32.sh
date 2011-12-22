#!/bin/sh

if [ -z $1 ]; then
  echo "Please specify bingo version";
  exit;
fi

if [ -z $2 ]; then
  echo "Please specify PostgreSQL version";
  exit;
fi

if [ -z $BINGO_PG_DIR32 ]; then
if [ -z $BINGO_PG_DIR ]; then
echo BINGO_PG_DIR is not specified
exit;
fi
export BINGO_PG_DIR32=$BINGO_PG_DIR
fi

make CONF=Release32
./bingo-release.sh $1 $2 dist/x86/bingo_postgres.so
