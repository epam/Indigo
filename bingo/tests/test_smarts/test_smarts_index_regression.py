import pytest
from sqlalchemy import text

from ..helpers import assert_match_query


class TestSmarts:
    @pytest.fixture(scope="class", autouse=True)
    def regression_table(self, db):
        if db.dbms != "postgres":
            yield None
            return

        table_name = "smarts_regression_m_m_t"
        db._execute_dml_query(
            f"DROP TABLE IF EXISTS {db.test_schema}.{table_name} CASCADE"
        )
        db.create_data_tables([table_name])
        db.create_indices([table_name])
        db._execute_dml_query(
            f"""
            INSERT INTO {db.test_schema}.{table_name}(id, data) VALUES
                (1, 'CCO'),
                (2, 'CSC'),
                (3, 'O=S=O'),
                (4, 'C1=CC=CC=C1'),
                (5, 'CC(=O)O')
            """
        )

        yield table_name

        db._connect.execute(text("COMMIT"))
        db._execute_dml_query(
            f"DROP TABLE IF EXISTS {db.test_schema}.{table_name} CASCADE"
        )

    def _assert_index_scan(self, db, table_name, condition_sql):
        db._connect.execute(text("SET enable_seqscan = off"))
        result = db._connect.execute(
            text(
                f"""
                EXPLAIN SELECT id FROM {db.test_schema}.{table_name}
                WHERE {condition_sql}
                """
            )
        )
        plan = "\n".join(row[0] for row in result.fetchall())
        result.close()
        db._connect.execute(text("COMMIT"))
        assert "Index Scan" in plan

    def _run_query(self, db, table_name, condition_sql):
        db._connect.execute(text("SET enable_seqscan = off"))
        result = db._connect.execute(
            text(
                f"""
                SELECT id FROM {db.test_schema}.{table_name}
                WHERE {condition_sql}
                ORDER BY id ASC
                """
            )
        )
        rows = [row[0] for row in result.fetchall()]
        result.close()
        db._connect.execute(text("COMMIT"))
        return rows

    def test_exact_indexed_regression(self, db, db_backend, regression_table):
        if db_backend != "postgres":
            pytest.skip("Regression test only supported in PostgreSQL backend")

        condition = "data @ CAST(('CSC', '') AS bingo.exact)"
        self._assert_index_scan(db, regression_table, condition)
        assert_match_query(self._run_query(db, regression_table, condition), [2])

    def test_substructure_indexed_regression(self, db, db_backend, regression_table):
        if db_backend != "postgres":
            pytest.skip("Regression test only supported in PostgreSQL backend")

        condition = "data @ CAST(('S', '') AS bingo.sub)"
        self._assert_index_scan(db, regression_table, condition)
        assert_match_query(self._run_query(db, regression_table, condition), [2, 3])

    def test_smarts_indexed_regression(self, db, db_backend, regression_table):
        if db_backend != "postgres":
            pytest.skip("Regression test only supported in PostgreSQL backend")

        condition = "data @ CAST(('[#16]', '') AS bingo.smarts)"
        self._assert_index_scan(db, regression_table, condition)
        assert_match_query(self._run_query(db, regression_table, condition), [2, 3])
