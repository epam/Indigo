version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

make CONF=Release32
make CONF=Release64

./release-unix.sh build_release/indigo-depict-$version-linux32 dist/Release32/GNU-Linux-x86/indigo-depict
./release-unix.sh build_release/indigo-depict-$version-linux64 dist/Release64/GNU-Linux-x86/indigo-depict
