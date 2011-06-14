#!/bin/sh

name=$1
libname=$2

if [ -z $name ]; then
  echo "specify name";
  exit;
fi

if [ -z $libname ]; then
  echo "specify library name";
  exit;
fi

rm -rf ./$name

mkdir -p $name/bin
mkdir -p $name/sql/bingo
mkdir -p $name/sql/system
 
cp LICENSE.GPL $name/
cp $libname $name/bin/
cp bingo-oracle-install.sh  $name/
cp sql/dbcheck.sql $name/sql/
cp sql/bingo/alter_routines.sql $name/sql/bingo/
cp sql/bingo/alter_routines2.sql $name/sql/bingo/
cp sql/bingo/bingo_calls.sql $name/sql/bingo/
cp sql/bingo/bingo_package.sql $name/sql/bingo/
cp sql/bingo/bingo_context.sql $name/sql/bingo/
cp sql/bingo/bingo_config.sql $name/sql/bingo/
cp sql/bingo/makebingo.sql $name/sql/bingo/
cp sql/bingo/dropbingo.sql $name/sql/bingo/
cp sql/bingo/mango_calls.sql $name/sql/bingo/
cp sql/bingo/mango_package.sql $name/sql/bingo/
cp sql/bingo/mango_drop.sql $name/sql/bingo/
cp sql/bingo/mango_index.sql $name/sql/bingo/
cp sql/bingo/mango_indextype.sql $name/sql/bingo/
cp sql/bingo/mango_make.sql $name/sql/bingo/
cp sql/bingo/mango_stat.sql $name/sql/bingo/
cp sql/bingo/ringo_calls.sql $name/sql/bingo/
cp sql/bingo/ringo_drop.sql $name/sql/bingo/
cp sql/bingo/ringo_index.sql $name/sql/bingo/
cp sql/bingo/ringo_indextype.sql $name/sql/bingo/
cp sql/bingo/ringo_make.sql $name/sql/bingo/
cp sql/bingo/ringo_package.sql $name/sql/bingo/
cp sql/bingo/ringo_stat.sql $name/sql/bingo/
cp sql/system/bingo_init.sql $name/sql/system/

zip -r -9 $name.zip $name
