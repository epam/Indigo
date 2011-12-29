version=$1

if [ -z $version ]; then
  echo "Please specify bingo version";
  exit;
fi

if [ -z $2 ]; then
  echo "Please specify PostgreSQL version";
  exit;
fi

./release-osx.10.5.sh bingo-postgres$2-$version-mac10.5 $2
./release-osx.10.6.sh bingo-postgres$2-$version-mac10.6 $2


