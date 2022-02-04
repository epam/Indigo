from os.path import abspath

TEST_CASES_DATA = {
    'mass': 'data/molecules/mass/std.json',
    'gross': 'data/molecules/gross/std.json',
    'fingerprint': 'data/molecules/fingerprint/std.json',
    'compactmolecule': 'data/molecules/compactmolecule/std.json',
    'inchi': 'data/molecules/inchi/std.json',
    'cml': 'data/molecules/cml/std.json',
    'exact': 'data/molecules/exact/std.json',
    'substructure': 'data/molecules/substructure/std.json',
    'similarity': 'data/molecules/similarity/std.json',
    'sgroups': 'data/molecules/sgroups/std.json',
    'smarts': 'data/molecules/smarts/std.json',
    'markush': 'data/molecules/markush/std.json',
    'resonance': 'data/molecules/resonance/std.json',
    'pseudoatoms': 'data/molecules/pseudoatoms/std.json',
    'bigtable': 'data/molecules/bigtable/std.json',
    'tautomers': 'data/molecules/tautomers/std.json',
    'checkmolecule': 'data/molecules/checkmolecule/std.json',
    'aam': 'data/reactions/aam/std.json',
    'checkreaction': 'data/reactions/checkreaction/std.json',
    'compactreaction': 'data/reactions/compactreaction/std.json',
    'rcml': 'data/reactions/rcml/std.json',
    'rexact': 'data/reactions/rexact/std.json',
    'rfingerprint': 'data/reactions/rfingerprint/std.json',
    'rsmarts': 'data/reactions/rsmarts/std.json',
    'rsmiles': 'data/reactions/rsmiles/std.json',
    'rsub': 'data/reactions/rsub/std.json',
}

QUERIES_DATA = {
    'mass': abspath('data/molecules/mass/import/queries/mols.sdf'),
    'gross': abspath('data/molecules/gross/import/queries/mols.sdf'),
    'fingerprint': abspath('data/molecules/fingerprint/import/queries/mols.sdf'),
    'compactmolecule': abspath('data/molecules/compactmolecule/import/queries/mols.sdf'),
    'inchi': abspath('data/molecules/inchi/import/queries/mols.sdf'),
    'cml': abspath('data/molecules/cml/import/queries/mols.sdf'),
    'exact': abspath('data/molecules/exact/import/queries/out.sdf'),
    'substructure': [
        abspath('data/molecules/substructure/import/queries/0_queries.sdf'),
        abspath('data/molecules/substructure/import/queries/1_explicit_valence_queries.smiles')
    ],
    'similarity': abspath('data/molecules/similarity/import/queries/queries.sdf'),
    'sgroups': abspath('data/molecules/sgroups/import/queries/out.sdf'),
    'smarts': abspath('data/molecules/smarts/import/queries/PaDEL_substructure.sma'),
    'markush': abspath('data/molecules/markush/import/queries/out.sdf'),
    'resonance': abspath('data/molecules/resonance/import/queries/queries.sdf'),
    'pseudoatoms': abspath('data/molecules/pseudoatoms/import/queries/out.sdf'),
    'bigtable': abspath('data/molecules/bigtable/import/queries/queries.smi'),
    'tautomers': abspath('data/molecules/tautomers/import/queries/queries.sdf'),
    'checkmolecule': abspath('data/molecules/checkmolecule/import/queries/mols.sdf'),
    'aam': abspath('data/reactions/aam/import/queries/rxns.rdf'),
    'checkreaction': abspath('data/reactions/checkreaction/import/queries/rxns.rdf'),
    'compactreaction': abspath('data/reactions/compactreaction/import/queries/rxns.rdf'),
    'rcml': abspath('data/reactions/rcml/import/queries/rxns.rdf'),
    'rexact': abspath('data/reactions/rexact/import/queries/exact_queries.rdf'),
    'rfingerprint': abspath('data/reactions/rfingerprint/import/queries/rxns.rdf'),
    'rsmarts': abspath('data/reactions/rsmarts/import/queries/queries.sma'),
    'rsmiles': abspath('data/reactions/rsmiles/import/queries/rxns.rdf'),
    'rsub': abspath('data/reactions/rsub/import/queries/queries.rdf'),
}

TARGET_TABLES_MAP = {
    'exact': 'exact_m_m_t',
    'tautomers': 'tautomers_m_m_t',
    'similarity': 'similarity_m_m_t',
    'substructure': 'substructure_m_m_t',
    'bigtable': 'bigtable_m_m_t',
    'markush': 'markush_m_m_t',
    'resonance': 'resonance_m_m_t',
    'pseudoatoms': 'pseudoatoms_m_m_t',
    'sgroups': 'sgroups_m_m_t',
    'smarts': 'smarts_m_m_t',
    'rexact': 'rexact_m_m_t',
    'rsmarts': 'rsmarts_m_m_t',
    'rsub': 'rsub_m_m_t'
}

CREATE_INDICES_FOR = ['exact', 'similarity', 'bigtable', 'smarts', 'tautomers',
                      'sgroups', 'markush', 'resonance', 'pseudoatoms']

MOL_FILES_EXTENSIONS = ['sdf', 'smi', '.sma', 'smiles', 'rdf']

IMPORT_FUNCTION_MAP = {
    '.sdf': 'importSDF',
    '.smi': 'importSMILES',
    '.smiles': 'importSMILES',
    '.sma': 'importSMILES',
    '.rdf': 'importRDF'
}

DB_POSTGRES = 'postgres'
DB_BINGO = 'bingo-nosql'
DB_BINGO_ELASTIC = 'bingo-elastic'
DB_ORACLE = 'oracle'
DB_MSSQL = 'mssql'

DATA_TYPES = {
    'aam': 'reactions',
    'bigtable': 'molecules',
    'checkmolecule': 'molecules',
    'checkreaction': 'reactions',
    'cml': 'molecules',
    'compactmolecule': 'reactions',
    'compactreaction': 'reactions',
    'exact': 'molecules',
    'fingerprint': 'molecules',
    'gross': 'molecules',
    'inchi': 'molecules',
    'markush': 'molecules',
    'mass': 'molecules',
    'pseudoatoms': 'molecules',
    'rcml': 'reactions',
    'resonance': 'molecules',
    'rexact': 'reactions',
    'rfingerprint': 'reactions',
    'rsmarts': 'reactions',
    'rsmiles': 'reactions',
    'rsub': 'reactions',
    'sgroups': 'molecules',
    'similarity': 'molecules',
    'smarts': 'molecules',
    'substructure': 'molecules',
    'tautomers': 'molecules',
}
