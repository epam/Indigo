#!/bin/sh

name=$1
pg_ver=$2
libname=$3

if [ -z $name ]; then
  echo "Please specify name";
  exit;
fi

if [ -z $pg_ver ]; then
  echo "Please specify PostgreSQL version";
  exit;
fi

if [ -z $libname ]; then
  echo "Please specify library name";
  exit;
fi

rm -rf ./$name

mkdir -p $name/bin
 
cp ../LICENSE.GPL $name/
cp README $name/
cp INSTALL $name/
cp $libname $name/bin/
cp bingo-pg-install.sh  $name/
cp -r sql/$pg_ver $name/

zip -r -9 $name.zip $name
