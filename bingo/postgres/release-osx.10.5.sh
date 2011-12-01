#!/bin/sh
reldir=`pwd`

xcodebuild -sdk macosx10.5 -configuration Release10.5 -alltargets
./bingo-release.sh $1 build/Release10.5/bingo_postgres.dylib
cd $reldir
