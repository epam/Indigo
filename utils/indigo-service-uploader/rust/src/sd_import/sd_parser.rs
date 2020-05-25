extern crate regex;

use std::collections::BTreeMap;
use rustc_serialize::json::{Json, ToJson};
use std::io::BufReader;
use std::io::Lines;
use std::io::prelude::*;
use flate2::Compression;
use flate2::write::GzEncoder;


static LINE_ENDING: char = '\n';

// Structure for keeping molecule and properties
pub struct SdItem {
    pub mol: Box<Vec<u8>>,
    pub props: Box<Json>,
}
/// SD files  parser 
pub struct SdParser<'a> {
    sd_iter: Lines<BufReader<&'a mut Read>>,
}

impl SdItem {
    pub fn new(sd: &str) -> Result<SdItem, String> {
        let re = regex::Regex::new(">[:space:]*<").unwrap();
        let mut has_mol = false;
        let mut mol: Option<String> = None;
        let mut prop_array: Vec<BTreeMap<String, Json>> = Vec::new();

        // Iterate properties. First in iteration is a molecule
        for cap in re.split(sd) {
            if has_mol {
                let prop_list = cap.trim();
                let prop_len = prop_list.len();
                let end_idx = prop_list.find(LINE_ENDING).unwrap_or(prop_len);
                let mut key_idx = end_idx;

                if end_idx > 0 {
                    key_idx -= 1;
                }

                let p_name = &cap[0..key_idx];

                let mut val_idx = end_idx;
                if val_idx + 1 < prop_len {
                    val_idx += 1;
                }
                let p_val = &cap[val_idx..prop_len];

                let mut props: BTreeMap<String, Json> = BTreeMap::new();

                // Create x,y,a,b representations
                props.insert("x".to_string(), p_name.to_string().to_lowercase().to_json());
                props.insert("y".to_string(), SdItem::read_property(p_val));
                props.insert("a".to_string(), p_name.to_string().to_json());
                props.insert("b".to_string(), p_val.to_string().to_json());

                prop_array.push(props);
            } else {
                mol = Some(cap.to_string());
                has_mol = true;
            }
        }
        let res = try!(mol.ok_or("no molecules").and_then(|sd| {

            // Gzip molecular structure
            let mut e = GzEncoder::new(Vec::new(), Compression::Default);
            e.write(sd.as_bytes()).ok().expect("Error while writing a molecule");
            let gz_mol = e.finish().unwrap();;

            Ok(SdItem {
                mol: Box::new(gz_mol),
                props: Box::new(prop_array.to_json()),
            })
        }));
        Ok(res)
    }

    fn read_property(p_val: &str) -> Json {
        match Json::from_str(p_val) {
            Ok(e) => e,
            Err(_) => p_val.to_string().to_json(),
        }
    }
}

impl<'a> SdParser<'a> {
    pub fn new(input: &'a mut Read) -> SdParser {
        let buf = BufReader::new(input);
        let iter = buf.lines();
        SdParser { sd_iter: iter }
    }
}

impl<'a> Iterator for SdParser<'a> {
    type Item = String;
    fn next(&mut self) -> Option<String> {
        let mut mol_str = String::new();

        // Iterate SD file by $$$$
        for mol in self.sd_iter
                       .by_ref()
                       .filter_map(|a| a.ok())
                       .take_while(|a| !a.starts_with("$$$$")) {
            mol_str.push_str(mol.as_ref());
            mol_str.push(LINE_ENDING);
        }
        return match mol_str.len() {
            0 => None,
            _ => Some(mol_str),
        };
    }
}

#[cfg(test)]
mod tests {
    static LINE_ENDING: char = '\n';
    use std::fs::File;
    use sd_parser::{SdItem, SdParser};
    use flate2::read::GzDecoder;
    use std::collections::BTreeSet;
    #[test]
    fn test_all_properties_parsed() {
        let sd_item = SdItem::new("975001
  -OEChem-05211109542D

 45 47  0     0  0  0  0  0  \
                                   0999 V2000
 25 45  1  0  0  0  0
M  END
>  <COMPOUND_CID>
\
                                   975001

> <COMPOUND_CANONICALIZED>
1

> <CACTVS_COMPLEXITY>
\
                                   426");

        let sd = &sd_item.unwrap();
        assert!(sd.props.is_array());
        let p_array = sd.props.as_array().unwrap();
        let mut sd_properties: BTreeSet<String> = BTreeSet::new();

        for p in p_array {
            let a = p.find("a").unwrap();
            sd_properties.insert(a.as_string().unwrap().to_string());
        }

        assert!(sd_properties.contains("COMPOUND_CANONICALIZED"));
        assert!(sd_properties.contains("COMPOUND_CID"));
        assert!(sd_properties.contains("CACTVS_COMPLEXITY"));
    }

    #[test]
    fn test_sd_parser_basic() {
        let mut f = File::open("../data/test_pubchem_10.sdf").unwrap();
        let parser = &mut SdParser::new(&mut f);
        assert_eq!(10, parser.count());
    }
    #[test]
    fn test_sd_parse_options() {
        let mut f = File::open("../data/test_pubchem_10.sdf").unwrap();
        let parser = &mut SdParser::new(&mut f);

        let sd = parser.next().unwrap();
        let sd_item = SdItem::new(sd.as_ref()).unwrap();

        assert!(sd_item.props.is_array());
        let p_array = sd_item.props.as_array().unwrap();
        for p in p_array {
            let a = p.find("a").unwrap().as_string().unwrap();
            let b = p.find("b").unwrap().as_string().unwrap();
            assert!(!b.starts_with(LINE_ENDING));

            if a.contains("PUBCHEM_EXACT_MASS") {
                assert!(p.find("y").unwrap().is_number());
            }
        }
    }

    #[test]
    fn test_basic_sd_iterator() {
        let mut f = File::open("../data/test_pubchem_10.sdf").unwrap();
        let parser = SdParser::new(&mut f);
        let mut p_size: usize = 0;
        for sd in parser {
            let sd_item = SdItem::new(sd.as_ref());
            let sd = &sd_item.unwrap();
            assert!(sd.props.is_array());
            p_size += sd.props.as_array().unwrap().len();
        }
        assert_eq!(325, p_size);
    }
    #[test]
    fn test_sd_scope_18() {
        let f = File::open("../data/test-18.sd.gz").unwrap();
        let mut f_str = GzDecoder::new(f).unwrap();
        let parser = SdParser::new(&mut f_str);
        let mut p_size: usize = 0;
        let mut m_size: usize = 0;
        for sd in parser {
            let sd_item = SdItem::new(sd.as_ref());
            let sd = &sd_item.unwrap();
            assert!(sd.props.is_array());
            p_size += sd.props.as_array().unwrap().len();
            m_size += 1;
        }
        assert_eq!(18, m_size);
        assert_eq!(576, p_size);
    }
    #[test]
    fn test_sd_scope_108() {
        let f = File::open("../data/test-108.sd.gz").unwrap();
        let mut f_str = GzDecoder::new(f).unwrap();
        let parser = SdParser::new(&mut f_str);
        let mut p_size: usize = 0;
        let mut m_size: usize = 0;
        for sd in parser {
            let sd_item = SdItem::new(sd.as_ref());
            let sd = &sd_item.unwrap();
            assert!(sd.props.is_array());
            p_size += sd.props.as_array().unwrap().len();
            m_size += 1;
        }
        assert_eq!(108, m_size);
        assert_eq!(3456, p_size);
    }
    #[test]
    fn test_sd_scope_2759() {
        let f = File::open("../data/test-2759.sd.gz").unwrap();
        let mut f_str = GzDecoder::new(f).unwrap();
        let parser = SdParser::new(&mut f_str);
        let mut p_size: usize = 0;
        let mut m_size: usize = 0;
        for sd in parser {
            let sd_item = SdItem::new(sd.as_ref());
            let sd = &sd_item.unwrap();
            assert!(sd.props.is_array());
            p_size += sd.props.as_array().unwrap().len();
            m_size += 1;
        }
        assert_eq!(2759, m_size);
        assert_eq!(8277, p_size);
    }
}
