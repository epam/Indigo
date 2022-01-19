import pytest
from indigo import Indigo

from .constants import (
    DB_BINGO,
    DB_BINGO_ELASTIC,
    DB_MSSQL,
    DB_ORACLE,
    DB_POSTGRES,
)
from .dbc.BingoNoSQL import BingoNoSQL
from .dbc.PostgresSQL import Postgres


@pytest.fixture(scope='class')
def indigo():
    return Indigo()


@pytest.fixture(scope='class')
def db(request, indigo):
    db_str = request.config.getoption("--db")
    if db_str == DB_POSTGRES:
        db = Postgres()
    elif db_str == DB_BINGO:
        db = BingoNoSQL(indigo)
        db.connect()
    elif db_str == DB_BINGO_ELASTIC:
        from .dbc.BingoElastic import BingoElastic
        db = BingoElastic(indigo)
    elif db_str == DB_ORACLE:
        pass
    elif db_str == DB_MSSQL:
        pass

    yield db

    db.close_connect()


def pytest_addoption(parser):
    parser.addoption("--db", action="store", default="default name")


# def pytest_generate_tests(metafunc):
#     # This is called for every test. Only get/set command line arguments# if the argument is specified in the list of test "fixturenames".
#     option_value = metafunc.config.option.name
#     if 'db' in metafunc.fixturenames and option_value is not None:
#         metafunc.parametrize("db", [option_value])
