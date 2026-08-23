# Local debug using VS Code

## Prerequisites 

* Copy 'launch.json' into '.vscode' folder on top of the indigo folder
* Copy 'CMakePresets.json' to indigo folder
* Install C++ extension

## Running local copy of PostgreSQL instance

Compile and install PostgreSQL
```
sudo apt-get install gettext
wget -P pg https://ftp.postgresql.org/pub/source/v12.6/postgresql-12.6.tar.gz
tar -xzvf postgresql-12.6.tar.gz
cd postgresql-12.6
./configure --without-readline --enable-nls
make -j $(nproc)
make install
```

Run db instance

```
/usr/local/pgsql/bin/initdb -D data/
/usr/local/pgsql/bin/pg_ctl -D data/ -l logfile start
```

Create user and database

```
/usr/local/pgsql/bin/createuser -s -w postgres
/usr/local/pgsql/bin/createdb -U postgres test
```

Build bingo-postgres target using 'Build' button
Make symbolic link if debug is required

```
cd bingo/lib
ln -s ~/indigo/build/bin/libbingo-postgres.so
```

Install bingo

```
cd bingo
sh bingo-pg-install.sh
/usr/local/pgsql/bin/psql -f bingo_install.sql test
```

Run debug icon in VS code (using indigo-core-unit-tests)
Run '(gdb) Attach' icon and connect to existing test instance
