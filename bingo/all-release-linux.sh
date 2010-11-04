version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi


make CONF=Release32
make CONF=Release64

./release-linux.32.sh bingo-oracle-$version-linux32
./release-linux.64.sh bingo-oracle-$version-linux64

