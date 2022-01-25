from abc import abstractmethod
from configparser import ConfigParser
from os import path
from os.path import abspath, join
from typing import Dict, List

import psycopg2
import sqlalchemy as sa
from indigo import IndigoException, IndigoObject
from indigo.bingo import BingoException
from sqlalchemy import text
from sqlalchemy.exc import (
    DatabaseError,
    InternalError,
    ProgrammingError,
    ResourceClosedError,
)

from ..constants import (
    DB_BINGO,
    DB_BINGO_ELASTIC,
    DB_MSSQL,
    DB_ORACLE,
    DB_POSTGRES,
    IMPORT_FUNCTION_MAP,
)
from ..logger import logger


def get_config():
    parser = ConfigParser()
    parser.read('db_config.ini')

    return {
        'common': {
            'bingo_schema': parser.get('common', 'bingo_schema'),
            'test_schema': parser.get('common', 'test_schema'),
        },
        DB_POSTGRES: {
            'host': parser.get(DB_POSTGRES, 'host'),
            'port': parser.get(DB_POSTGRES, 'port'),
            'database': parser.get(DB_POSTGRES, 'database'),
            'user': parser.get(DB_POSTGRES, 'user'),
            'password': parser.get(DB_POSTGRES, 'password')
        },
        DB_ORACLE: None,
        DB_MSSQL: None,
        DB_BINGO: {
            'db_name': parser.get(DB_BINGO, 'db_name'),
            'db_dir': parser.get(DB_BINGO, 'db_dir')
        },
        DB_BINGO_ELASTIC: {
            'host': parser.get(DB_BINGO_ELASTIC, 'host'),
            'port': parser.get(DB_BINGO_ELASTIC, 'port')
        }
    }


class DBAdapter:
    dbms = None
    config = None

    def __init__(self):
        self.config = get_config()
        self._set_db_config()
        # self.molecules_data_path = MOLECULES_DATA_PATH
        # self.reactions_data_path = REACTIONS_DATA_PATH

    @abstractmethod
    def _set_db_config(self):
        pass

    @abstractmethod
    def checkmolecule(self, molecule: IndigoObject):
        pass

    @abstractmethod
    def cml(self, molecule: IndigoObject):
        pass

    @abstractmethod
    def compactmolecule(self, molecule: IndigoObject):
        pass

    @abstractmethod
    def fingerprint(self, molecule: IndigoObject, options):
        pass

    @abstractmethod
    def gross(self, molecule: IndigoObject):
        pass

    @abstractmethod
    def inchi(self, molecule: IndigoObject, options='', inchikey=False):
        pass

    @abstractmethod
    def mass(self, molecule: IndigoObject, options):
        pass

    @abstractmethod
    def exact(self, molecule: IndigoObject, target_function: str, options=''):
        pass

    @abstractmethod
    def similarity(self, molecule: IndigoObject, target_function: str, options):
        pass

    @abstractmethod
    def smarts(self, molecule: IndigoObject, target_function: str, options=''):
        pass

    @abstractmethod
    def substructure(self, molecule: IndigoObject,
                     target_function: str, options=''):
        pass

    @abstractmethod
    def close_connect(self):
        pass


class NoSQLAdapter(DBAdapter):
    db_name = None
    db_dir = None
    db_path = None
    indigo = None
    bingo = None

    def _set_db_config(self):
        self.db_name = abspath(self.config[self.dbms]['db_name'])
        self.db_dir = abspath(self.config[self.dbms]['db_dir'])
        self.db_path = join(self.db_dir, self.db_name)


class SQLAdapter(DBAdapter):
    _connect = None
    _engine = None
    _session = None

    def _set_db_config(self):
        dbms_config = self.config[self.dbms]
        self.bingo_schema = self.config['common']['bingo_schema']
        self.test_schema = self.config['common']['test_schema']
        self.host = dbms_config['host']
        self.port = dbms_config['port']
        self.database = dbms_config['database']
        self.user = dbms_config['user']
        self.password = dbms_config['password']

    @property
    def conn_string(self):
        conn_string = '{dialect}+{driver}://{user}:{password}' \
                      '@{host}:{port}/{database}'
        if self.dbms == 'postgres':
            dialect, driver = 'postgresql', 'psycopg2'
        if self.dbms == 'oracle':
            pass
        if self.dbms == 'mssql':
            pass

        return conn_string.format(
            dialect=dialect,
            driver=driver,
            user=self.user,
            password=self.password,
            port=self.port,
            host=self.host,
            database=self.database
        )

    def _select_error_text(self, exception: Exception,
                           errors_start_with: List[str], delimeter: str,
                           error_ends_with=None):
        """
        Retrieve clean error text from raised Exception and wrap it with
        common Exception
        """
        for line in str(exception).split(delimeter):
            for starts_with in errors_start_with:
                if line.find(starts_with) != -1:
                    result = str(
                        ':'.join(line.split(':')[-2:])[1:]).replace("\\'", "'")
                    if error_ends_with:
                        end_pos = result.find(error_ends_with)
                        if end_pos != -1:
                            result = result[0:end_pos]
                    return Exception(result)
        raise exception

    def _execute_query(self, query: str, entity: IndigoObject,
                       table_name: str, options: str):
        """
        Execute query and return list of rows.
        In case of Excepton return new Exception with clean error message
        """
        query = query.format(
            test_schema=self.test_schema,
            bingo_schema=self.bingo_schema,
            query_entity=entity.rawData(),
            table_name=table_name,
            options=options
        )
        rows = None

        try:
            result = self._connect.execute(text(query))
            if not result.closed:
                rows = result.fetchall()
        except (DatabaseError, InternalError, psycopg2.InternalError,
                ProgrammingError, Exception) as e:
            errors_start_with = [
                '(InternalError) error: ',
                'bingo:',
                'ERROR:  error: bingo:',
                '(psycopg2.errors.InternalError_) error: bingo:',
                'psycopg2.errors.SyntaxError'
            ]
            return self._select_error_text(e, errors_start_with, '\n')

        return rows

    def _execute_dml_query(self, query: str):
        rows = None
        connect = self._connect
        t = connect.begin()
        try:
            result = connect.execute(query)
            if not result.closed:
                try:
                    rows = result.fetchall()
                except ResourceClosedError:
                    pass
            t.commit()
        except (DatabaseError, InternalError, psycopg2.InternalError) as e:
            t.rollback()
        return rows, query

    def create_data_tables(self, tables: List[str]):
        created_tables = []
        sa_meta = sa.MetaData()
        self.engine.execute(text(f"CREATE SCHEMA IF NOT EXISTS {self.test_schema}"))
        for table in tables:
            logger.debug(f"Creating {self.dbms} table {table}")
            created_tables.append(
                sa.Table(
                    table, sa_meta,
                    # Every data table contains the same columns: id and data columns
                    sa.Column('id', sa.Integer, primary_key=True),
                    sa.Column('data', sa.Text, nullable=True),
                    schema=self.test_schema
                )
            )
        sa_meta.create_all(self.engine)

        return created_tables

    def import_data(self, import_meta: Dict[str, str], other_columns=''):
        for table, import_path in import_meta.items():
            import_path_ext = path.splitext(import_path)[1]
            function = IMPORT_FUNCTION_MAP.get(import_path_ext)
            logger.debug(
                f'Importing data to table {table} from src: {import_path}')
            import_path = import_path.replace('\\', '/')
            dml_query = "SELECT {bingo}.{function}('{test_schema}.{table}', " \
                        "'data', '{other_columns}', '{file}')"
            dml_query = dml_query.format(bingo=self.bingo_schema,
                                         function=function,
                                         test_schema=self.test_schema,
                                         table=table,
                                         other_columns=other_columns,
                                         file=import_path)
            self._execute_dml_query(dml_query)

    def create_indices(self, tables: List[str]):
        """
        Creates index for every provided table
        """
        for table in tables:
            logger.debug(f"Creating index {self.test_schema}_{table}_idx")
            dml_query = "CREATE INDEX {test_schema}_{table}_idx ON " \
                        "{test_schema}.{table} " \
                        "USING bingo_idx (data {bingo_schema}.molecule)"
            dml_query = dml_query.format(
                test_schema=self.test_schema,
                table=table,
                bingo_schema=self.bingo_schema
            )
            self._execute_dml_query(dml_query)

    @property
    def engine(self):
        return self._engine

    @property
    def session(self):
        return self._session

    def close_connect(self):
        logger.debug(f"Closing connection to {self.dbms}")
        self._connect.close()


def catch_indigo_exception(catch_error=False):
    def decorate_method(method):
        def wrapper(self, *args, **kwargs):
            try:
                return method(self, *args, **kwargs)
            except (BingoException, IndigoException) as e:
                replace_list = [
                    'indigo.bingo.BingoException:',
                    'indigo.IndigoException:',
                    '\''
                ]
                if catch_error:
                    msg = str(e)
                    for replacement in replace_list:
                        msg = msg.replace(replacement, '')
                    return Exception(msg)
                return None

        return wrapper

    return decorate_method
