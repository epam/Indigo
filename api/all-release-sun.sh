#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../graph
make -f Makefile.sun.32
make -f Makefile.sun.64
cd ../molecule
make -f Makefile.sun.32
make -f Makefile.sun.64
cd ../reaction
make -f Makefile.sun.32
make -f Makefile.sun.64
cd ../layout
make -f Makefile.sun.32
make -f Makefile.sun.64
cd ../api
make -f Makefile.sun.32
make -f Makefile.sun.64

cd jni
make -f Makefile.sun.32
make -f Makefile.sun.64
cd ..

./indigo-libs-release-sun.sh indigo-libs-$1-sun32 32
./indigo-libs-release-sun.sh indigo-libs-$1-sun64 64
./indigo-java-release-sun.sh indigo-java-api-$1-sun

