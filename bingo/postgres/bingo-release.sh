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
 
cp ../LICENSE.GPL $name/
cp README $name/
cp INSTALL $name/
cp $libname $name/bin/
cp bingo-pg-install.sh  $name/
cp -r sql $name/

zip -r -9 $name.zip $name
