#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

make CONF=ReleaseStatic32
make CONF=ReleaseStatic64
make CONF=ReleaseShared32
make CONF=ReleaseShared64

cd renderer
make CONF=ReleaseStatic32
make CONF=ReleaseStatic64
make CONF=ReleaseShared32
make CONF=ReleaseShared64

cd ../jni
make CONF=Release32
make CONF=Release64

cd ../renderer/jni
make CONF=Release32
make CONF=Release64

cd ../..
./indigo-libs-release-linux.sh indigo-libs-$1-linux32 32
./indigo-libs-release-linux.sh indigo-libs-$1-linux64 64
./indigo-java-release-linux.sh indigo-java-api-$1-linux
./indigo-python-release-linux.sh indigo-python-api-$1-linux

