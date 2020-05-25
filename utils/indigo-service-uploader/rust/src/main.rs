extern crate getopts;
extern crate sd_import;

use sd_import::SdImport;

use getopts::Options;
use std::env;

fn import(file_name: &str, table_name: &str, config_name: Option<String>) {
    let conf_name = config_name.unwrap_or("config.yml".to_string());
    let mut sd_import = SdImport::new(&conf_name);
    sd_import.insert(file_name, table_name);
}

fn print_usage(program: &str, opts: Options) {
    let brief = format!("Usage: {} [options] <file_path> <table_name>", program);
    print!("{}", opts.usage(&brief));
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let program = args[0].clone();

    let mut opts = Options::new();
    opts.optopt("c",
                "config-file",
                "path to config file (optional, default is 'config.yml')",
                "CONFIG");
    opts.optflag("h", "help", "print this help menu");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => panic!(f.to_string()),
    };
    if matches.opt_present("h") {
        print_usage(&program, opts);
        return;
    }

    let (input, table_name) = if matches.free.len() > 1 {
        (matches.free[0].clone(), matches.free[1].clone())
    } else {
        print_usage(&program, opts);
        return;
    };
    //let t_name = matches.opt_str("t");
    //let table_name = t_name.expect("No table name is specified");
    let config_name = matches.opt_str("c");
    import(&input, &table_name, config_name);
}
