#!/bin/sh

name=$1
bits=$2

if [ -z $name ]; then
  echo "specify name";
  exit;
fi
if [ -z $bits ]; then
  echo "specify bits";
  exit;
fi

rm -rf ./$name 

mkdir -p $name

cp LICENSE.GPL $name/
cp indigo.h $name/
cp sparc"$bits"static/libindigo.a $name
cp sparc"$bits"shared/libindigo.so $name
cp ../graph/sparc$bits/libgraph.a $name
cp ../molecule/sparc$bits/libmolecule.a $name
cp ../reaction/sparc$bits/libreaction.a $name
cp ../layout/sparc$bits/liblayout.a $name

zip -r -9 $name.zip $name
