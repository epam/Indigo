#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../../tinyxml
make -f Makefile.sun.32
make -f Makefile.sun.64
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
cd ../utils/indigo-deco
make -f Makefile.sun.32
make -f Makefile.sun.64
./release-unix.sh indigo-deco-$version-sun32 sparc32/indigo-deco
./release-unix.sh indigo-deco-$version-sun64 sparc64/indigo-deco
