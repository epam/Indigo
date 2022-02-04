import pytest
from indigo import Indigo

from .constants import (
    DATA_TYPES,
    DB_BINGO,
    DB_BINGO_ELASTIC,
    DB_MSSQL,
    DB_ORACLE,
    DB_POSTGRES
)
from .dbc.BingoElastic import BingoElastic
from .dbc.BingoNoSQL import BingoNoSQL
from .dbc.PostgresSQL import Postgres
from .helpers import get_bingo_meta, get_query_entities
from .logger import logger


@pytest.fixture(scope='class')
def indigo():
    # TODO: uncomment this:
    indigo = Indigo()
    # indigo.setOption("ignore-stereochemistry-errors", True)
    # indigo.setOption("ignore-bad-valence", True)
    return indigo


@pytest.fixture(scope='class')
def entities(request, indigo):
    function = request.node.name.replace('Test', '').lower()
    entities = get_query_entities(indigo, function)
    yield entities
    del entities


@pytest.fixture(scope='class')
def db(request, indigo):
    db_str = request.config.getoption("--db")
    function = request.node.name.replace('Test', '').lower()
    data_type = DATA_TYPES[function]
    logger.info(f"===== Start of testing {function} =====")
    meta = get_bingo_meta(function, data_type)
    if db_str == DB_POSTGRES:
        db = Postgres()
        pg_tables = db.create_data_tables(meta['tables'])
        db.import_data(import_meta=meta['import'])
        db.create_indices(meta['indices'])
    elif db_str == DB_BINGO:
        db = BingoNoSQL(indigo)
        db.connect()
        db.import_data(meta['import_no_sql'], data_type)
    elif db_str == DB_BINGO_ELASTIC:
        db = BingoElastic(indigo)
        db.import_data(meta['import_no_sql'], data_type)
    elif db_str == DB_ORACLE:
        pass
    elif db_str == DB_MSSQL:
        pass

    yield db

    logger.info("Dropping DB...")
    if db_str == DB_POSTGRES:
        for table in pg_tables:
            logger.info(f'Dropping Postgres table {table}')
            table.drop(db.engine)
    elif db_str == DB_BINGO:
        db.delete_base()
    elif db_str == DB_BINGO_ELASTIC:
        db.drop()
    logger.info(f"===== Finish of testing {function} =====")


def pytest_addoption(parser):
    parser.addoption("--db", action="store", default="default name")


# @pytest.fixture(scope='class')
# def db(request, indigo):
#     db_str = request.config.getoption("--db")
#     if db_str == DB_POSTGRES:
#         db = Postgres()
#     elif db_str == DB_BINGO:
#         db = BingoNoSQL(indigo)
#         db.connect()
#     elif db_str == DB_BINGO_ELASTIC:
#         db = BingoElastic(indigo)
#     elif db_str == DB_ORACLE:
#         pass
#     elif db_str == DB_MSSQL:
#         pass
#
#     yield db
#
#     db.close_connect()


# def pytest_generate_tests(metafunc):
#     # This is called for every test. Only get/set command line arguments# if the argument is specified in the list of test "fixturenames".
#     option_value = metafunc.config.option.name
#     if 'db' in metafunc.fixturenames and option_value is not None:
#         metafunc.parametrize("db", [option_value])
