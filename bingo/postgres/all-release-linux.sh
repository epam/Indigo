version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi
	
./release-linux.32.sh bingo-postgres-$version-linux32
./release-linux.64.sh bingo-postgres-$version-linux64

