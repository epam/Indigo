extern crate postgres;
extern crate time;
extern crate num_cpus;
extern crate threadpool;
extern crate flate2;
extern crate yaml_rust;
extern crate postgres_binary_copy;
extern crate rustc_serialize;

mod sd_batch_uploader;
mod sd_parser;

use postgres::{Connection, SslMode, ConnectParams, ConnectTarget, UserInfo};
use std::collections::BTreeMap;
use time::PreciseTime;
use std::sync::mpsc;
use std::thread;
use threadpool::ThreadPool;
use std::io::prelude::*;
use std::fs::File;
use flate2::read::GzDecoder;
use sd_batch_uploader::SdBatchUploader;
use sd_parser::{SdParser, SdItem};
use yaml_rust::{Yaml, YamlLoader};

/// Main Library for loading SD files into database
/// Reads configuration file and performs upload
pub struct SdImport {
    pub db_config: BTreeMap<String, String>,
    pub general_config: BTreeMap<String, yaml_rust::yaml::Yaml>,
    pub db_conn: Connection,
}

fn read_config(config_name: &str) -> (BTreeMap<String, String>, BTreeMap<String, yaml_rust::yaml::Yaml>) {
    let mut f_config = File::open(config_name)
                           .ok()
                           .expect(&format!("Can not open configuration file '{}'", config_name));
    let mut s = String::new();
    f_config.read_to_string(&mut s)
            .ok()
            .expect(&format!("Error while reading configuration file '{}'", config_name));
    let conf = YamlLoader::load_from_str(&s).unwrap();

    let mut db_config: BTreeMap<String, String> = BTreeMap::new();
    let mut general_config: BTreeMap<String, yaml_rust::yaml::Yaml> = BTreeMap::new();

    for c in conf {
        let h = c.as_hash().unwrap();

        let db = h.get(&Yaml::String("database".to_string())).unwrap();
        let db_conf = db.as_hash().unwrap();

        for (k, v) in db_conf {
            db_config.insert(k.as_str().unwrap().to_string(),
                             v.as_str().unwrap().to_string());
        }
        let general = h.get(&Yaml::String("general".to_string())).expect("no general section in config");
        let general_conf = general.as_hash().unwrap();

        for (k, v) in general_conf {
            general_config.insert(k.as_str().unwrap().to_string(),
                             v.clone());
        }
    }
    (db_config, general_config)
}



impl<'a> SdImport {
    pub fn new(config_name: &str) -> SdImport {
        let (db_conf, general_conf) = read_config(config_name);

        let params = ConnectParams {
            target: ConnectTarget::Tcp(db_conf.get("url").unwrap().clone()),
            port: Some(5432),
            user: Some(UserInfo {
                user: db_conf.get("user").unwrap().clone(),
                password: Some(db_conf.get("pass").unwrap().clone()),
            }),
            database: Some(db_conf.get("db").unwrap().clone()),
            options: vec![],
        };

        let conn = Connection::connect(params, &SslMode::None).unwrap();
        SdImport {
            db_config: db_conf,
            general_config: general_conf,
            db_conn: conn,
        }
    }
    pub fn insert(&mut self, file_name: &str, table_name: &str) {
        let t_name = format!("{}.{}", self.db_config.get("schema").unwrap(), table_name);
        self.parallel_insert(file_name, &t_name)
    }
    fn parallel_insert(&mut self, file_name: &str, table_name: &str) {

        // TODO: move creating table into config
        // let drop_stmt = format!("drop table if exists {}", table_name);
        // let create_stmt = format!("create table {} (id serial, m bytea, p jsonb) ", table_name);
        // conn.execute(&drop_stmt, &[]).ok().expect("Table drop failed");
        // conn.execute(&create_stmt, &[]).ok().expect("Table creation failed");

        println!("Start import");

        let start_t = PreciseTime::now();
        let mut str_count: u32 = 0;
        let buf_size: usize = self.general_config.get("buffer_size").unwrap().as_i64().unwrap() as usize;
        let (map_send, map_rec) = mpsc::sync_channel(buf_size);
        let (reduce_send, reduce_rec) = mpsc::sync_channel(buf_size);
        let reduce_sender = reduce_send.clone();
        let f_name = file_name.to_string();

        // Create parallel reader
        let sd_reader = thread::spawn(move || {
            let pool = ThreadPool::new(num_cpus::get());
            let f = File::open(f_name).unwrap();
            let mut f_str = GzDecoder::new(f).unwrap();
            let parser = SdParser::new(&mut f_str);

            for sd in parser {
                reduce_sender.send(1u8).unwrap();
                let map_sender = map_send.clone();
                pool.execute(move || {
                    let sd_item = SdItem::new(&sd).unwrap();
                    map_sender.send(sd_item).unwrap();
                });
                str_count += 1;
            }
            reduce_sender.send(0u8).unwrap();
        });

        // Start upload into database

        let sd_uploader = &mut SdBatchUploader::new(&self.db_conn, table_name).unwrap();
        loop {
            let status = reduce_rec.recv().unwrap();
            match status {
                1u8 => {
                    let sd_item: SdItem = map_rec.recv().unwrap();
                    sd_uploader.upload(sd_item);
                    str_count += 1;
                }
                _ => break,
            }
        }

        sd_reader.join().unwrap();

        let end_t = PreciseTime::now();
        let timer_ms = start_t.to(end_t).num_milliseconds() as f32 ;
        let timer_s = timer_ms / 1000f32;

        println!("Insert total time = {} ms ", timer_ms as i32);
        println!("Average insert time = {} structures per second", ((str_count as f32) / timer_s) as i32);
        println!("Total structures processed = {}", str_count);

    }

    // fn single_insert() {
    // {
    //     let sd_uploader = &mut SdBatchUploader::new(&conn, table_name).unwrap();
    //     let f = File::open("data/test-108.sd.gz").unwrap();
    //     let mut f_str = GzDecoder::new(f).unwrap();
    //     let parser = SdParser::new(&mut f_str);
    //     for sd in parser {
    //         let sd_item = SdItem::new(sd.as_ref());
    //         let sd = sd_item.unwrap();
    //         sd_uploader.upload(sd);
    //     }
    // }
    // }
}
