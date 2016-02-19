## Prerequisites 

You should specify the PostgreSQL directory by setting the special environment variables: BINGO_PG_DIR, BINGO_PG_VERSION. 

```
	export BINGO_PG_DIR=`<path-to-postgres>`
	export BINGO_PG_VERSION=`<version>`
```
For example
```
	export BINGO_PG_DIR=/var/lib/postgresql
	export BINGO_PG_VERSION=9.5
```

## Build

Run python installation script

```
python build_scripts/bingo-release.py --preset=linux64 --dbms=postgres
```

## Build Debug:
  

  On Linux (minimum required gcc version 4.7)


	cd <path-to-indigo>/build_scripts/bingo-postgres
	cmake .
	make

  On Windows, (Microsoft Visual Studio 2013 is required)

	set BINGO_PG_DIR=<path-to-postgres>
	set BINGO_PG_VERSION=9.2
	cd <path-to-indigo>/build_scripts/bingo-postgres
	cmake-gui .
	<change the compiler>
	<press configure>
	<run the solution file by visual studio>




