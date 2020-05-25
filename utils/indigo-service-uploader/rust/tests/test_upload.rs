extern crate postgres;
extern crate flate2;
extern crate sd_import;


use postgres::types::FromSql;
use postgres::Connection;
use postgres::Result as PgResult;
use sd_import::SdImport;

fn get_single_value<T>(conn: &Connection, query: &str) -> PgResult<T>
    where T: FromSql
{
    println!("Executing query: {}", query);
    let stmt = try!(conn.prepare(query));
    let rows = try!(stmt.query(&[]));
    let row = rows.iter().next().unwrap();
    row.get_opt(0)
}

fn drop_create_table(conn: &Connection, table_name: &str) {
    let drop_stmt = format!("drop table if exists {}", table_name);
    let create_stmt = format!("create table {} (id serial, m bytea, p jsonb) ", table_name);

    conn.execute(&drop_stmt, &[]).ok().expect("Table drop failed");
    conn.execute(&create_stmt, &[]).ok().expect("Table creation failed");
}

fn get_query_count(conn: &Connection, query: String) -> i64 {
    get_single_value::<i64>(&conn, &query).unwrap()
}

#[test]
fn test_basic_upload() {
    let mut sd_import = SdImport::new("tests/config.yml");

    let table_name = "test_indigo_upload";

    // Test basic upload
    drop_create_table(&sd_import.db_conn, table_name);

    sd_import.insert("../data/test-108.sd.gz", table_name);

    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count (*) from {}", table_name));
        assert_eq!(108, t_count);
    }

    // Test pubchem insert
    drop_create_table(&sd_import.db_conn, table_name);

    sd_import.insert("../data/test_pubchem_10.sd.gz", table_name);

    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count(*) from {} \
                                               ,jsonb_array_elements(p) elems where elems->>'x' \
                                               like '%mass%' and (elems->>'y')::float > 300",
                                              table_name));
        assert_eq!(6, t_count);
    }
    // Test maybridge
    drop_create_table(&sd_import.db_conn, table_name);

    sd_import.insert("../data/maybridge-stardrop-sample.sd.gz", table_name);

    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count(*) from {} \
                                               ,jsonb_array_elements(p) elems where elems->>'x' like '%logp%' and (elems->>'y')::float > 5",
                                              table_name));
        assert_eq!(16, t_count);
    }
    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count (*) from {}", table_name));
        assert_eq!(108, t_count);
    }

    // Test floats
    drop_create_table(&sd_import.db_conn, table_name);

    sd_import.insert("../data/test-18-floats.sd.gz", table_name);

    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count(*) from {} \
                                               ,jsonb_array_elements(p) elems where elems->>'x' like '%logs%' and (elems->>'y')::float > 0.5",
                                              table_name));
        assert_eq!(36, t_count);
    }
    {
        let t_count = get_query_count(&sd_import.db_conn,
                                      format!("select count(*) from {} \
                                               ,jsonb_array_elements(p) elems where elems->>'x' like '%logs%' and (elems->>'y')::float > 1",
                                              table_name));
        assert_eq!(26, t_count);
    }
}
