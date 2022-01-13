## Prerequisites 

Download, compile and install PostgreSQL 

```
wget -P pg https://ftp.postgresql.org/pub/source/v12.6/postgresql-12.6.tar.gz
tar -xzvf postgresql-12.6.tar.gz
cd postgresql-12.6
./configure --without-readline --enable-nls
make
make install
```

## Build

Run cmake build script

```
mkdir build
cd build
cmake .. -DBUILD_BINGO_POSTGRES=ON -DBUILD_INDIGO=OFF -DBUILD_INDIGO_WRAPPERS=OFF -DBUILD_BINGO_SQLSERVER=Off -DBUILD_BINGO_ORACLE=OFF -DBUILD_BINGO_ELASTIC=OFF -DBUILD_INDIGO_UTILS=OFF -DPostgreSQL_ROOT=/usr/local/pgsql
cmake --build . --config Release --target package-bingo-postgres -- -j $(nproc)
```

