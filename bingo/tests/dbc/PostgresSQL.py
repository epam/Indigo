from psycopg2 import InternalError

from indigo import IndigoObject
from sqlalchemy.engine import create_engine
from sqlalchemy.orm.session import sessionmaker

from ..constants import DB_POSTGRES, TARGET_TABLES_MAP
from ..logger import logger
from .base import SQLAdapter

MATCHING_SEARCH_QUERY = "SELECT id, 1 from {test_schema}.{table_name} " \
                        "WHERE data @ ('{query_entity}', '{options}') :: " \
                        "{bingo_schema}.{function} ORDER BY ID ASC"


class Postgres(SQLAdapter):
    dbms = DB_POSTGRES

    def __init__(self):
        SQLAdapter.__init__(self)
        logger.debug(f"Opening connection to {self.dbms}")
        self._engine = create_engine(self.conn_string, convert_unicode=True)
        session = sessionmaker(bind=self._engine)
        session.configure(bind=self._engine, autocommit=False,
                          autoflush=False,
                          _enable_transaction_accounting=False)
        self._session = session()
        self._session.dialect = self._engine.dialect
        self._connect = self._engine.connect()

    def query_row(self, query: str, entity: IndigoObject,
                  table_name='', options=''):
        """Execute SQL query and return single row"""
        try:
            rows = self._execute_query(query, entity, table_name, options)
            if rows:
                return rows[0][0]
            return rows
        except Exception as e:
            return e

    def query_rows(self, query: str, entity: IndigoObject,
                   table_name='', options=''):
        """Execute SQL query and return a list of rows"""
        try:
            rows = self._execute_query(query, entity, table_name, options)
            if type(rows) is list:
                rows = [row[0] for row in rows]

            return rows
        except Exception as e:
            return e

    def checkmolecule(self, molecule):
        query_sql = "SELECT {bingo_schema}.checkmolecule(" \
                    "'{query_entity}')"
        return self.query_row(query_sql, molecule)

    def cml(self, molecule):
        query_sql = "SELECT {bingo_schema}.cml('{query_entity}')"
        return self.query_row(query_sql, molecule)

    def compactmolecule(self, molecule):
        query_sql = "SELECT md5({bingo_schema}.compactmolecule(" \
                    "'{query_entity}', '{options}'))"
        return self.query_row(query_sql, molecule, options='0')

    def fingerprint(self, molecule, options):
        query_sql = "SELECT md5({bingo_schema}.fingerprint(" \
                    "'{query_entity}', '{options}'))"
        return self.query_row(query_sql, molecule, options=options)

    def gross(self, molecule):
        query_sql = "SELECT {bingo_schema}.gross('{query_entity}')"
        return self.query_row(query_sql, molecule)

    def inchi(self, molecule, options='', inchikey=False):
        query_sql = "SELECT {bingo_schema}.inchi(" \
                    "'{query_entity}', '{options}')"
        if inchikey:
            query_sql = "SELECT {bingo_schema}.inchikey({bingo_schema}." \
                        "inchi('{query_entity}', ' '))"
        return self.query_row(query_sql, molecule, options=options)

    def mass(self, molecule, options):
        query_sql = "SELECT {bingo_schema}.getWeight(" \
                    "'{query_entity}', '{options}')"
        return self.query_row(query_sql, molecule, options=options)

    def similarity(self, molecule, target_function, options):
        table_name = TARGET_TABLES_MAP.get(target_function)
        sim_type, min_sim, max_sim = options.split(', ')
        query_sql = "SELECT id, {bingo_schema}.getsimilarity(data, " \
                    "'{query_entity}', '{sim_type}') FROM " \
                    "{test_schema}.{table_name} WHERE data @ ({min_sim}, " \
                    "{max_sim}, '{query_entity}', '{sim_type}') :: " \
                    "{bingo_schema}.sim ORDER BY id ASC"
        query_sql = query_sql.replace('{sim_type}', sim_type)
        query_sql = query_sql.replace('{min_sim}', min_sim)
        query_sql = query_sql.replace('{max_sim}', max_sim)
        return self.query_rows(query_sql, molecule, table_name, options)

    def exact(self, molecule, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'exact')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def substructure(self, molecule, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'sub')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def smarts(self, molecule, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'sub')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, molecule, table_name, options)

    def aam(self, reaction, options):
        query_sql = "SELECT {bingo_schema}.aam('{query_entity}', '{options}')"
        return self.query_row(query_sql, reaction, options=options)

    def checkreaction(self, reaction, options=''):
        query_sql = "SELECT {bingo_schema}.checkreaction('{query_entity}')"
        return self.query_row(query_sql, reaction, options=options)

    def compactreaction(self, reaction, options='0'):
        query_sql = "SELECT md5({bingo_schema}.compactreaction(" \
                    "'{query_entity}', False))"
        return self.query_row(query_sql, reaction, options=options)

    def rcml(self, reaction, options=''):
        query_sql = "SELECT {bingo_schema}.rcml('{query_entity}')"
        return self.query_row(query_sql, reaction, options=options)

    def rfingerprint(self, reaction, options=''):
        query_sql = "SELECT md5({bingo_schema}.rfingerprint('{" \
                    "query_entity}', '{options}'))"
        return self.query_row(query_sql, reaction, options=options)

    def rsmiles(self, reaction, options=''):
        query_sql = "SELECT {bingo_schema}.rsmiles('{query_entity}')"
        return self.query_row(query_sql, reaction, options=options)

    def rexact(self, reaction, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'rexact')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)

    def rsmarts(self, reaction, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'rsmarts')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)

    def rsubstructure(self, reaction, target_function, options=''):
        query_sql = MATCHING_SEARCH_QUERY.replace('{function}', 'rsub')
        table_name = TARGET_TABLES_MAP.get(target_function)
        return self.query_rows(query_sql, reaction, table_name, options)
