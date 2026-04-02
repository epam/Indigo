from os import path

import sqlalchemy as sa
from indigo import IndigoObject
from sqlalchemy import text
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

MATCHING_SEARCH_QUERY = (
    "SELECT id, 1 from {test_schema}.{table_name} "
    "WHERE {bingo_schema}.{function}(data, :query_entity{params_clause})=1 "
    "ORDER BY ID ASC"
)


class Oracle(SQLAdapter):
    dbms = DB_ORACLE

    def __init__(self):
        SQLAdapter.__init__(self)
        logger.debug(f"Opening connection to {self.dbms}")
        self._engine = create_engine(self.conn_string)
        session = sessionmaker(bind=self._engine)
        session.configure(
            bind=self._engine,
            autocommit=False,
            autoflush=False,
        )
        self._session = session()
        self._session.dialect = self._engine.dialect
        self._connect = self._engine.connect()

    def _execute_query(self, query, entity, table_name, options):
        query = query.format(
            test_schema=self.test_schema,
            bingo_schema=self.bingo_schema,
            table_name=table_name,
            options=options,
        )
        rows = None
        try:
            result = self._connect.execute(
                text(query), {"query_entity": entity.rawData()}
            )
            if not result.closed:
                rows = result.fetchall()
        except (DatabaseError, InternalError, ProgrammingError, Exception) as e:
            errors_start_with = [
                "ORA-",
                "bingo:",
                "(oracledb.exceptions.",
            ]
            return self._select_error_text(e, errors_start_with, "\n")
        return rows

    def create_data_tables(self, tables):
        created_tables = []
        sa_meta = sa.MetaData()
        for table in tables:
            logger.debug(f"Creating {self.dbms} table {table}")
            created_tables.append(
                sa.Table(
                    table,
                    sa_meta,
                    sa.Column("id", sa.Integer, primary_key=True),
                    sa.Column(
                        "data", sa.Text, nullable=True
                    ),
                    schema=self.test_schema,
                )
            )
        sa_meta.create_all(self.engine)
        return created_tables

    def import_data(self, import_meta, other_columns=""):
        for table, import_path in import_meta.items():
            import_path_ext = path.splitext(import_path)[1]
            function = IMPORT_FUNCTION_MAP.get(import_path_ext)
            logger.debug(
                f"Importing data to table {table} from src: {import_path}"
            )
            for item in function(import_path):
                query = (
                    "INSERT INTO {test_schema}.{table}(data) "
                    "VALUES (:item_data)"
                )
                query = query.format(
                    test_schema=self.test_schema,
                    table=table,
                )
                connect = self._connect
                t = connect.begin()
                connect.execute(
                    text(query),
                    {"item_data": item.rawData()},
                )
                t.commit()

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

    def query_row(
        self, query: str, entity: IndigoObject, table_name="", options=""
    ):
        """Execute SQL query and return single row"""
        try:
            rows = self._execute_query(query, entity, table_name, options)
            if rows:
                return rows[0][0]
            return rows
        except Exception as e:
            return e

    def query_rows(
        self, query: str, entity: IndigoObject, table_name="", options=""
    ):
        """Execute SQL query and return a list of rows"""
        try:
            rows = self._execute_query(query, entity, table_name, options)
            if type(rows) is list:
                rows = [row[0] for row in rows]

            return rows
        except Exception as e:
            return e

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
        query_sql = (
            "SELECT dbms_crypto.hash("
            "{bingo_schema}.CompactMolecule(:query_entity, 0), 2) FROM dual"
        )
        return self.query_row(query_sql, molecule)

    def fingerprint(self, molecule, options):
        query_sql = (
            "SELECT dbms_crypto.hash("
            "{bingo_schema}.Fingerprint(:query_entity, '{options}'), 2) "
            "FROM dual"
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
        query_sql = (
            "SELECT {bingo_schema}.Mass(:query_entity, '{options}') FROM dual"
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
        query_sql = (
            "SELECT id, 1 from {test_schema}.{table_name} "
            "WHERE {bingo_schema}.Smarts(data, :query_entity)=1 "
            "ORDER BY ID ASC"
        )
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def aam(self, reaction, options):
        query_sql = (
            "SELECT {bingo_schema}.AutoAAM(:query_entity, '{options}') "
            "FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def checkreaction(self, reaction, options=""):
        query_sql = (
            "SELECT {bingo_schema}.CheckReaction(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def compactreaction(self, reaction, options="0"):
        query_sql = (
            "SELECT dbms_crypto.hash("
            "{bingo_schema}.CompactReaction(:query_entity, 0), 2) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rcml(self, reaction, options=""):
        query_sql = (
            "SELECT {bingo_schema}.RCML(:query_entity) FROM dual"
        )
        return self.query_row(query_sql, reaction, options=options)

    def rfingerprint(self, reaction, options=""):
        query_sql = (
            "SELECT dbms_crypto.hash("
            "{bingo_schema}.RFingerprint(:query_entity, '{options}'), 2) "
            "FROM dual"
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
        query_sql = (
            "SELECT id, 1 from {test_schema}.{table_name} "
            "WHERE {bingo_schema}.RSmarts(data, :query_entity)=1 "
            "ORDER BY ID ASC"
        )
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)

    def rsubstructure(self, reaction, target_function, options=""):
        params_clause = f", '{options}'" if options else ""
        query_sql = MATCHING_SEARCH_QUERY.replace(
            "{function}", "RSub"
        ).replace("{params_clause}", params_clause)
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)
