#!/bin/sh
# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

libdir=$PWD/lib
schema_name="bingo"
libext=".so"
y="0"
pglibdir="0"


bingo_pg_name="libbingo-postgres"

if [ -f "lib/libbingo-postgres.dylib" ]; then
  libext=".dylib"
  bingo_pg_name="libbingo-postgres.dylib"
fi

usage ()
{
echo 'Usage: bingo-pg-install.sh [parameters]'
echo 'Parameters:'
echo '  -?, -help'
echo '    Print this help message'
echo '  -libdir path'
echo "    Target directory with the installed bingo-postgres$libext (default {PWD}/lib)."
echo '  -schema name'
echo '    Postgres schema name (default "bingo").'
echo '  -pglibdir'
echo '    Use PostgreSQL $libdir option (default "false")'
echo "    Notice: bingo-postgres$libext must be placed in the package library directory"
echo '  -y'
echo '    Process default options (default "false")'
}

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
        exit 255
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
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo_schema.sql.in  >bingo_install.sql
sed 's,BINGO_PATHNAME,'$libdir'/'$bingo_pg_name',g' <sql/bingo_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo_pg.sql.in  >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/'$bingo_pg_name',g' <sql/mango_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/mango_pg.sql.in       >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/'$bingo_pg_name',g' <sql/ringo_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/ringo_pg.sql.in       >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/'$bingo_pg_name',g' <sql/bingo_am.sql.in     >>bingo_install.sql
sed 's,BINGO_PATHNAME,'$libdir'/'$bingo_pg_name',g' <sql/bingo_config.sql.in >>bingo_install.sql

#Generate uninstall script
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo_uninstall.quick.sql.in >bingo_uninstall.sql
