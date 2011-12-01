version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi


./release-osx.10.5.sh bingo-postgres-$version-mac10.5
./release-osx.10.6.sh bingo-postgres-$version-mac10.6


