import config
import psycopg2
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import scoped_session, sessionmaker

pg_conf = config.__dict__.get("BINGO_POSTGRES")


def connect(self):
    return psycopg2.connect(**pg_conf)


engine = create_engine("postgresql://", creator=connect, convert_unicode=True)

db_session = scoped_session(
    sessionmaker(autocommit=False, autoflush=False, bind=engine)
)
Base = declarative_base()
Base.query = db_session.query_property()
