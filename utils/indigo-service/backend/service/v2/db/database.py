import psycopg2  # type: ignore
from sqlalchemy import create_engine  # type: ignore
from sqlalchemy.ext.declarative import declarative_base  # type: ignore
from sqlalchemy.orm import scoped_session, sessionmaker  # type: ignore

from ..common.config import BINGO_POSTGRES


def connect():
    return psycopg2.connect(**BINGO_POSTGRES)


engine = create_engine("postgresql://", creator=connect, convert_unicode=True)

db_session = scoped_session(
    sessionmaker(autocommit=False, autoflush=False, bind=engine)
)
Base = declarative_base()
Base.query = db_session.query_property()
