#!/bin/sh
version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

./make-all.sun.32.sh
./make-all.sun.64.sh

./release-sun-32.sh bingo-oracle-$version-sun32
./release-sun-64.sh bingo-oracle-$version-sun64

