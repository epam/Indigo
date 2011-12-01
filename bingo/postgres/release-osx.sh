#!/bin/sh
reldir=`pwd`

xcodebuild -sdk macosx10.6 -configuration Release10.6 -alltargets
./bingo-release.sh $1 build/Release10.6/bingo_postgres.dylib
cd $reldir
