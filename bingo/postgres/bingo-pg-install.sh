#!/bin/sh
# Copyright (C) 2009-2011 GGA Software Services LLC
# 
# This file is part of Indigo toolkit.
# 
# This file may be distributed and/or modified under the terms of the
# GNU General Public License version 3 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

libdir=$PWD/bin
schema_name="bingo"
libext=".so"
y="0"
pglibdir="0"
usage ()
{
echo 'Usage: bingo-pg-install.sh [parameters]'
echo 'Parameters:'
echo '  -?, -help'
echo '    Print this help message'
echo '  -libdir path'
echo '    Target directory to install bingo_postgres'$libext' (defaut {CURRENT_DIR}/bin/).'
echo '    If the directory does not exist, it will be created.'
echo '  -schema name'
echo '    Postgres schema name (default "bingo").'
echo '  -pglibdir'
echo '    Use postgreSQL $libdir option (default "false")'
echo '    Notice: bingo_postgres'$libext' must be placed in the package library directory'
echo '  -y'
echo '    Process default options (default "false")'
}


#if [ -f "../bin/libbingo.dylib" ]; then
#  libext=".dylib"
#fi

while [ "$#" != 0 ]; do
  case "$1" in
     -help | '-?' | '/?')
        usage
        exit 0
        ;;
     -libdir)
        shift
        libdir=$1
        ;;
     -schema)
        shift
        schema_name=$1
        ;;
     -y)
        y="1"
        ;;
     -pglibdir)
        pglibdir="1"
        ;;
     *)
        echo "Unknown parameter: $1";
        usage;
        exit -1
  esac
  shift
done

if [ "$pglibdir" != "0" ]; then
  libdir='$libdir';
fi

echo "Target directory  : $libdir";
echo "DBA schema name   : $schema_name";

if [ "$y" != "1" ]; then
  echo "Proceed (y/N)?"
  read proceed

  if [ "$proceed" != "y" ] && [ "$proceed" != "Y" ]; then
    echo 'Aborting';
    exit 0;
  fi
fi


# Generate install script
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/bingo_schema.sql.in  >bingo_install.sql
sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/bingo_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/bingo_pg.sql.in  >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/mango_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/mango_pg.sql.in       >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/ringo_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/ringo_pg.sql.in       >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/bingo_am.sql.in     >>bingo_install.sql
sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/bingo_config.sql.in >>bingo_install.sql

#Generate uninstall script
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/bingo_uninstall.quick.sql.in >bingo_uninstall.sql


