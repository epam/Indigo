use sd_parser::SdItem;
use postgres::Connection;
use postgres::types::{Type, ToSql};

use postgres_binary_copy::BinaryCopyReader;

static MAX_CAPACITY: u64 = 1 << 23; // 8 MB

/// Postgres Batch Uploader
/// Uses a cache and binary copy
pub struct SdBatchUploader<'a> {
    conn: &'a Connection,
    buf: Vec<Box<ToSql>>,
    copy_stmt: String,
    buf_size: u64,
}

impl<'a> SdBatchUploader<'a> {
    pub fn new(pg_conn: &'a Connection, table_name: &str) -> Result<SdBatchUploader<'a>, String> {
        Ok(SdBatchUploader {
            conn: pg_conn,
            buf: Vec::new(),
            copy_stmt: format!("COPY {} (m, p) FROM STDIN BINARY", table_name),
            buf_size: 0,
        })
    }

    pub fn upload(&mut self, sd_item: SdItem) {
        self.buf_size += sd_item.mol.as_ref().len() as u64;
        // for (key, value) in sd_item.props.iter() {
        //     self.buf_size += key.len() as u64;
        //     self.buf_size += value.len() as u64;
        // }
        self.buf.push(sd_item.mol);
        self.buf.push(sd_item.props);
        if self.buf_size > MAX_CAPACITY {
            self.flush();
        }
    }

    fn flush(&mut self) {
        if self.buf.len() == 0 {
            return;
        }
        {
            let types = &[Type::Bytea, Type::Jsonb];
            let data = self.buf.iter().map(|v| &**v);
            let mut reader = BinaryCopyReader::new(types, data);

            let stmt = self.conn.prepare(&self.copy_stmt).unwrap();
            stmt.copy_in(&[], &mut reader).unwrap();
        }

        self.buf.clear();
        self.buf_size = 0;
    }
}


impl<'a> Drop for SdBatchUploader<'a> {
    fn drop(&mut self) {
        self.flush();
    }
}
