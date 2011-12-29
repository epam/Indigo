version=$1

if [ -z $version ]; then
  echo "Please specify bingo version";
  exit;
fi

if [ -z $2 ]; then
  echo "Please specify PostgreSQL version";
  exit;
fi
	
./release-linux.32.sh bingo-postgres$2-$version-linux32 $2
./release-linux.64.sh bingo-postgres$2-$version-linux64 $2

