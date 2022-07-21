import pytest
from indigo import Indigo
from .constants import (
    DATA_TYPES,
    DB_BINGO,
    DB_BINGO_ELASTIC,
    DB_MSSQL,
    DB_ORACLE,
    DB_POSTGRES,
    EntitiesType,
)
from .dbc.BingoNoSQL import BingoNoSQL
from .dbc.PostgresSQL import Postgres
from .helpers import get_bingo_meta, get_query_entities
from .logger import logger


@pytest.fixture(scope="class")
def indigo():
    # TODO: uncomment this:
    indigo = Indigo()
    return indigo


@pytest.fixture(scope="class")
def entities(request, indigo):
    function = request.node.name.replace("Test", "").lower()
    entities = get_query_entities(indigo, function)
    yield entities
    del entities


@pytest.fixture(scope="class")
def db(request, indigo):
    db_str = request.config.getoption("--db")
    function = request.node.name.replace("Test", "").lower()
    data_type = DATA_TYPES[function]
    logger.info(f"===== Start of testing {function} =====")
    meta = get_bingo_meta(function, data_type.value)
    if db_str == DB_POSTGRES:
        db = Postgres()
        pg_tables = db.create_data_tables(meta["tables"])
        db.import_data(import_meta=meta["import"])
        db.create_indices(meta["indices"])
    elif db_str == DB_BINGO:
        db = BingoNoSQL(indigo)
        db.connect()
        db.import_data(meta["import_no_sql"], data_type)
    elif db_str == DB_BINGO_ELASTIC:
        from bingo_elastic.elastic import IndexName
        from .dbc.BingoElastic import BingoElastic

        if data_type == EntitiesType.MOLECULES:
            index_name = IndexName.BINGO_MOLECULE
        else:
            index_name = IndexName.BINGO_REACTION
        db = BingoElastic(indigo, index_name)
        db.import_data(meta["import_no_sql"], data_type)
    elif db_str == DB_ORACLE:
        pass
    elif db_str == DB_MSSQL:
        pass

    yield db

    logger.info("Dropping DB...")
    db.close_connect()
    if db_str == DB_POSTGRES:
        for table in pg_tables:
            logger.info(f"Dropping Postgres table {table}")
            table.drop(db.engine)
    elif db_str == DB_BINGO:
        db.delete_base()
    elif db_str == DB_BINGO_ELASTIC:
        db.drop()
    logger.info(f"===== Finish of testing {function} =====")


def pytest_addoption(parser):
    parser.addoption("--db", action="store", default="default name")
