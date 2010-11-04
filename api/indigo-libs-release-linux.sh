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
cp renderer/indigo-renderer.h $name
cp dist/ReleaseStatic$bits/GNU-Linux-x86/libindigo.a $name
cp dist/ReleaseShared$bits/GNU-Linux-x86/libindigo.so $name
cp renderer/dist/ReleaseStatic$bits/GNU-Linux-x86/libindigo-renderer.a $name
cp renderer/dist/ReleaseShared$bits/GNU-Linux-x86/libindigo-renderer.so $name
cp ../graph/dist/Release$bits/GNU-Linux-x86/libgraph.a $name
cp ../molecule/dist/Release$bits/GNU-Linux-x86/libmolecule.a $name
cp ../reaction/dist/Release$bits/GNU-Linux-x86/libreaction.a $name
cp ../layout/dist/Release$bits/GNU-Linux-x86/liblayout.a $name
cp ../render2d/dist/Release$bits/GNU-Linux-x86/librender2d.a $name

zip -r -9 $name.zip $name
