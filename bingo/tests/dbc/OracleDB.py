import sys
import time
from os import path

# python-oracledb is the successor to cx_Oracle. SQLAlchemy 1.3.22 (pinned in
# bingo/tests/requirements.txt) does not ship the oracle+oracledb dialect
# (added in SQLAlchemy 2.0), so we register oracledb as cx_Oracle here — the
# officially documented compat path — and keep using oracle+cx_oracle://.
# See https://python-oracledb.readthedocs.io/en/latest/user_guide/appendix_b.html
import oracledb

oracledb.version = "8.3.0"
sys.modules.setdefault("cx_Oracle", oracledb)

import sqlalchemy as sa
from indigo import IndigoObject
from sqlalchemy import event
from sqlalchemy.engine import create_engine
from sqlalchemy.exc import DatabaseError, InternalError, ProgrammingError
from sqlalchemy.orm.session import sessionmaker

from ..constants import (
    DB_ORACLE,
    IMPORT_FUNCTION_MAP,
    TARGET_TABLES_MAP,
)
from ..logger import logger
from .base import SQLAdapter

from sqlalchemy.dialects.oracle import CLOB

MATCHING_SEARCH_QUERY = (
    "SELECT id, 1 from {test_schema}.{table_name} "
    "WHERE {bingo_schema}.{function}(data, :query_entity{params_clause})=1 "
    "ORDER BY ID ASC"
)

class _RawDataEntity:
    def __init__(self, data):
        self._data = data

    def rawData(self):
        return self._data


class Oracle(SQLAdapter):
    dbms = DB_ORACLE

    def __init__(self):
        SQLAdapter.__init__(self)
        logger.debug(f"Opening connection to {self.dbms}")
        self._engine = create_engine(self.conn_string)
        bingo_schema = self.bingo_schema

        @event.listens_for(self._engine, "connect")
        def _init_bingo_context(dbapi_connection, connection_record):
            cur = dbapi_connection.cursor()
            try:
                cur.callproc(
                    f"{bingo_schema}.ConfigSetInt",
                    [0, "ct_format_save_date", 0],
                )
                # ConfigSetInt does DELETE+INSERT on CONFIG_INT; commit
                # immediately so the row lock isn't held for the life of
                # the connection and doesn't block concurrent test sessions.
                dbapi_connection.commit()
            finally:
                cur.close()

        session = sessionmaker(bind=self._engine)
        session.configure(
            bind=self._engine,
            autocommit=False,
            autoflush=False,
            _enable_transaction_accounting=False,
        )
        self._session = session()
        self._session.dialect = self._engine.dialect
        self._connect = self._engine.connect()

    # ORA-28579 = extproc agent died mid-call (the OS process behind the
    # bingo .so). It restarts automatically on the next call, so a short
    # retry recovers without test-level intervention.
    _RETRYABLE_ORA_CODES = ("ORA-28579",)

    def _execute_query(self, query, entity, table_name, options):
        query_sql = query.format(
            test_schema=self.test_schema,
            bingo_schema=self.bingo_schema,
            table_name=table_name,
            options=options,
        )
        raw_data = entity.rawData()
        if isinstance(raw_data, bytes):
            raw_data = raw_data.decode("utf-8", errors="strict")

        stmt = sa.text(query_sql).bindparams(
            sa.bindparam("query_entity", type_=CLOB)   # force CLOB bind
        )

        errors_start_with = ["bingo:", "ORA-", "(oracledb.exceptions."]
        for attempt in range(3):
            try:
                result = self._connect.execute(
                    stmt, {"query_entity": raw_data}
                )
                return result.fetchall() if not result.closed else None
            except (DatabaseError, InternalError, ProgrammingError, Exception) as e:
                msg = str(e)
                if (
                    attempt < 2
                    and any(code in msg for code in self._RETRYABLE_ORA_CODES)
                ):
                    time.sleep(0.5)
                    continue
                return self._select_error_text(e, errors_start_with, "\n")

    # Oracle wraps RAISE_APPLICATION_ERROR(-20XXX,...) in an outer ORA-29902
    # ("error in executing ODCIIndexStart()") plus ORA-06512 PL/SQL backtrace
    # frames. The inner ORA-20XXX line contains the actual bingo message
    # (e.g. "ORA-20352: Error: element: bad valence on H..."). The base impl
    # would return the outer wrapper. Skip those wrapper frames and strip the
    # leading "Error: " bingo emits ahead of the real text.
    _WRAPPER_ORA_CODES = ("ORA-29902", "ORA-06512", "ORA-29400")

    def _select_error_text(
        self, exception, errors_start_with, delimeter, error_ends_with=None
    ):
        candidate_lines = []
        for line in str(exception).split(delimeter):
            if any(code in line for code in self._WRAPPER_ORA_CODES):
                continue
            if line.startswith(("Help:", "[SQL:", "[parameters:")):
                continue
            candidate_lines.append(line)

        for starts_with in errors_start_with:
            for line in candidate_lines:
                if line.find(starts_with) != -1:
                    # .lstrip() drops the leading space introduced by the
                    # ":".join(...) collapse in the typical bingo error
                    # ("...bingo: Error: msg" → " element: msg" → "element: msg").
                    # A hard [1:] also strips legitimate first characters
                    # of 2-colon lines, e.g. "ORA-28579: ..." → "RA-28579: ...".
                    result = ":".join(line.split(":")[-2:]).lstrip().replace(
                        "\\'", "'"
                    )
                    if result.startswith("Error: "):
                        result = result[len("Error: "):]
                    if error_ends_with:
                        end_pos = result.find(error_ends_with)
                        if end_pos != -1:
                            result = result[0:end_pos]
                    return Exception(result)
        raise exception

    def query_row(
        self, 
        query: str, 
        entity: IndigoObject, 
        table_name="", 
        options=""):
        res = self._execute_query(query, entity, table_name, options)

        if isinstance(res, Exception):
            return res

        if isinstance(res, str):
            if "bingo:" in res.lower() or "valence" in res.lower():
                return Exception(res)
            return res

        if not res:
            return None

        val = res[0][0]

        # Oracle can return CLOB as a LOB object; read it
        if hasattr(val, "read"):
            return val.read()

        return val

    def query_rows(
        self, 
        query: str, 
        entity: IndigoObject, 
        table_name="",
        options=""):
        res = self._execute_query(query, entity, table_name, options)

        if isinstance(res, Exception):
            return res

        if not isinstance(res, list):
            return res

        result = []
        for row in res:
            val = row[0]
            # Oracle can return CLOB as a LOB object; read it
            if hasattr(val, "read"):
                val = val.read()
            result.append(val)

        return result

    def create_data_tables(self, tables):
        created_tables = []
        sa_meta = sa.MetaData()
        for table in tables:
            created_tables.append(
                sa.Table(
                    table,
                    sa_meta,
                    sa.Column("id", sa.Integer, sa.Sequence(f"{table}_id_seq", start=1), primary_key=True),
                    sa.Column("data", CLOB, nullable=True),  # force CLOB
                    schema=self.test_schema,
                )
            )
        sa_meta.create_all(self.engine)
        return created_tables

    def import_data(self, import_meta, other_columns=""):
        for table, import_path in import_meta.items():
            function = IMPORT_FUNCTION_MAP.get(path.splitext(import_path)[1])
            for item in function(import_path):
                # Oracle sequences don't auto-fire on text()-bound INSERTs the
                # way Postgres SERIAL does — the sequence is wired to the
                # column via SQLAlchemy metadata but raw SQL must call NEXTVAL
                # explicitly, otherwise id binds to NULL and ORA-01400 fires.
                sql = (
                    f"INSERT INTO {self.test_schema}.{table}(id, data) "
                    f"VALUES ({table}_id_seq.NEXTVAL, :item_data)"
                )
                stmt = sa.text(sql).bindparams(sa.bindparam("item_data", type_=CLOB))

                data = item.rawData()
                if isinstance(data, bytes):
                    data = data.decode("utf-8", errors="ignore")

                tx = self._connect.begin()
                try:
                    self._connect.execute(stmt, {"item_data": data})
                    tx.commit()
                except Exception:
                    tx.rollback()
                    raise

    def create_indices(self, tables):
        for table in tables:
            logger.debug(
                f"Creating index {self.test_schema}_{table}_idx"
            )
            dml_query = (
                "CREATE INDEX {test_schema}_{table}_idx ON "
                "{test_schema}.{table}(data) "
                "INDEXTYPE IS {bingo_schema}.MoleculeIndex"
            )
            dml_query = dml_query.format(
                test_schema=self.test_schema,
                table=table,
                bingo_schema=self.bingo_schema,
            )
            self._execute_dml_query(dml_query)

    def checkmolecule(self, molecule):
        query_sql = (
            "SELECT {bingo_schema}.CheckMolecule(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, molecule)

    def cml(self, molecule):
        query_sql = (
            "SELECT {bingo_schema}.CML(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, molecule)

    def compactmolecule(self, molecule):
        # dbms_crypto.hash() returns a RAW that oracledb surfaces as Python
        # bytes; tests compare against the lowercase hex digest that Postgres'
        # digest()::text / encode(..., 'hex') produces. LOWER(RAWTOHEX(...))
        # aligns the two without adapter-side byte conversion.
        #
        # Postgres' digest(NULL, 'md5') returns NULL; Oracle's dbms_crypto.hash
        # treats NULL (or an empty BLOB) as empty and returns MD5("") =
        # d41d8cd9…e. Test data encodes "couldn't compact" as expected=None —
        # and CompactMolecule returns a zero-length BLOB (not NULL) in that
        # case, so LENGTH() is the correct guard (IS NULL is insufficient).
        query_sql = (
            "SELECT CASE WHEN c IS NULL OR LENGTH(c) = 0 THEN NULL "
            "ELSE LOWER(RAWTOHEX(dbms_crypto.hash(c, 2))) END "
            "FROM (SELECT {bingo_schema}.CompactMolecule(:query_entity, 0) AS c FROM dual)"
        )
        return self.query_row(query_sql, molecule)

    def fingerprint(self, molecule, options):
        # See compactmolecule() for rationale on LOWER(RAWTOHEX(...)) and the
        # NULL-passthrough CASE — same Postgres/Oracle hash-of-NULL disparity.
        query_sql = (
            "SELECT CASE WHEN f IS NULL OR LENGTH(f) = 0 THEN NULL "
            "ELSE LOWER(RAWTOHEX(dbms_crypto.hash(f, 2))) END "
            "FROM (SELECT {bingo_schema}.Fingerprint(:query_entity, '{options}') AS f FROM dual)"
        )
        return self.query_row(query_sql, molecule, options=options)

    def gross(self, molecule):
        query_sql = (
            "SELECT {bingo_schema}.Gross(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, molecule)

    def inchi(self, molecule, options="", inchikey=False):
        query_sql = (
            "SELECT {bingo_schema}.InChI(:query_entity, '{options}') FROM dual"
        )
        if inchikey:
            query_sql = (
                "SELECT {bingo_schema}.InChIKey("
                "{bingo_schema}.InChI(:query_entity, ' ')) FROM dual"
            )
        return self.query_row(query_sql, molecule, options=options)

    def mass(self, molecule, options):
        # Mass() returns NUMBER → oracledb maps to Decimal; helpers.assert_calculate_query
        # uses `type(result) is float`, so Decimal falls through to exact equality and fails.
        query_sql = (
            "SELECT CAST({bingo_schema}.Mass(:query_entity, '{options}') "
            "AS BINARY_DOUBLE) FROM dual"
        )
        return self.query_row(query_sql, molecule, options=options)

    def similarity(self, molecule, target_function, sim_type, options=""):
        table_name = TARGET_TABLES_MAP.get(target_function)
        min_sim, max_sim = options.split(", ")
        query_sql = (
            "SELECT id, {bingo_schema}.Sim(data, :query_entity, "
            "'{sim_type}') FROM "
            "{test_schema}.{table_name} WHERE "
            "{bingo_schema}.Sim(data, :query_entity, '{sim_type}') "
            "BETWEEN {min_sim} AND {max_sim} ORDER BY id ASC"
        )
        query_sql = query_sql.replace("{sim_type}", sim_type)
        query_sql = query_sql.replace("{min_sim}", min_sim)
        query_sql = query_sql.replace("{max_sim}", max_sim)
        return self.query_rows(query_sql, molecule, table_name, options)

    def exact(self, molecule, target_function, options=""):
        params_clause = f", '{options}'" if options else ""
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "Exact"
        ).replace("{params_clause}", params_clause)
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def substructure(self, molecule, target_function, options=""):
        params_clause = f", '{options}'" if options else ""
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "Sub"
        ).replace("{params_clause}", params_clause)
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def smarts(self, molecule, target_function, options=""):
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "Smarts"
        ).replace("{params_clause}", "")
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def aam(self, reaction, options):
        query_sql = (
            "SELECT {bingo_schema}.aam(:query_entity, '{options}') "
            "FROM dual"
        )
        try:
            rxn_data = reaction.rxnfile()
        except Exception:
            rxn_data = reaction.rawData()

        return self.query_row(query_sql, _RawDataEntity(rxn_data), options=options)

    def checkreaction(self, reaction, options=""):
        query_sql = (
            "SELECT {bingo_schema}.CheckReaction(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def compactreaction(self, reaction, options="0"):
        query_sql = (
            "SELECT CASE WHEN c IS NULL OR LENGTH(c) = 0 THEN NULL "
            "ELSE LOWER(RAWTOHEX(dbms_crypto.hash(c, 2))) END "
            "FROM (SELECT {bingo_schema}.CompactReaction(:query_entity, 0) AS c FROM dual)"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rcml(self, reaction, options=""):
        query_sql = (
            "SELECT {bingo_schema}.RCML(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rfingerprint(self, reaction, options=""):
        query_sql = (
            "SELECT CASE WHEN f IS NULL OR LENGTH(f) = 0 THEN NULL "
            "ELSE LOWER(RAWTOHEX(dbms_crypto.hash(f, 2))) END "
            "FROM (SELECT {bingo_schema}.RFingerprint(:query_entity, '{options}') AS f FROM dual)"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rsmiles(self, reaction, options=""):
        query_sql = (
            "SELECT {bingo_schema}.RSMILES(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rexact(self, reaction, target_function, options=""):
        params_clause = f", '{options}'" if options else ""
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "RExact"
        ).replace("{params_clause}", params_clause)
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)

    def rsmarts(self, reaction, target_function, options=""):
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "RSmarts"
        ).replace("{params_clause}", "")
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)

    def rsubstructure(self, reaction, target_function, options=""):
        params_clause = f", '{options}'" if options else ""
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "RSub"
        ).replace("{params_clause}", params_clause)
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)
