#!/bin/bash
# First-init hook (gvenzl /container-entrypoint-initdb.d): install bingo + create test user.
set -euo pipefail

INSTANCE="localhost:1521/${ORACLE_DATABASE:-XEPDB1}"

echo "[bingo] Installing cartridge against ${INSTANCE}"
cd /opt/bingo-oracle
sh ./bingo-oracle-install.sh \
  -libdir /opt/bingo-lib \
  -dbaname system \
  -dbapass "${ORACLE_PASSWORD}" \
  -instance "${INSTANCE}" \
  -bingoname bingo \
  -bingopass bingo \
  -y

echo "[bingo] Creating test/test schema user"
sqlplus -s "system/${ORACLE_PASSWORD}@${INSTANCE}" <<'SQL'
WHENEVER SQLERROR EXIT SQL.SQLCODE
BEGIN EXECUTE IMMEDIATE 'DROP USER test CASCADE'; EXCEPTION WHEN OTHERS THEN NULL; END;
/
CREATE USER test IDENTIFIED BY test DEFAULT TABLESPACE bingo;
GRANT CONNECT TO test;
GRANT CREATE TABLE TO test;
GRANT CREATE SESSION TO test;
GRANT CREATE SEQUENCE TO test;
GRANT UNLIMITED TABLESPACE TO test;
GRANT EXECUTE ON bingo.MangoPackage TO test;
GRANT EXECUTE ON bingo.RingoPackage TO test;
GRANT EXECUTE ON bingo.BingoPackage TO test;
BEGIN
  FOR obj IN (
    SELECT object_name
    FROM dba_objects
    WHERE owner = 'BINGO'
      AND object_type IN ('FUNCTION','PROCEDURE','TYPE','INDEXTYPE')
      AND status = 'VALID'
  ) LOOP
    BEGIN
      EXECUTE IMMEDIATE 'GRANT EXECUTE ON bingo.' || obj.object_name || ' TO test';
    EXCEPTION WHEN OTHERS THEN NULL;
    END;
  END LOOP;
END;
/
EXIT;
SQL

sqlplus -s "sys/${ORACLE_PASSWORD}@${INSTANCE} AS SYSDBA" <<'SQL'
WHENEVER SQLERROR EXIT SQL.SQLCODE
GRANT EXECUTE ON sys.dbms_crypto TO test;
EXIT;
SQL

echo "[bingo] Cartridge installed and test/test user created"
