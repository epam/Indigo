#!/bin/sh

name=$1
exename=$2

if [ -z $name ]; then
  echo "specify name";
  exit;
fi

if [ -z $exename ]; then
  echo "specify executable name";
  exit;
fi

rm -rf ./$name

mkdir -p $name
 
cp ../../api/LICENSE.GPL $name/
cp $exename $name/

zip -r -9 $name.zip $name

