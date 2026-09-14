# Glossary

> **Read when:** a term in a ticket, a test name, or a class name is opaque.
> **Skip when:** you are not blocked on vocabulary — this file is a lookup table, not a briefing.

| Term | Meaning | Where it shows up |
| --- | --- | --- |
| AAM | Atom-to-atom mapping between reactants and products | `reaction/`, `indigoAutomap` |
| Aromatize / Dearomatize | Convert between aromatic bond typing and alternating single/double bonds | `molecule/molecule_arom*.h` |
| Bingo | The chemistry cartridge: molecular indexing and search inside a database | `bingo/` |
| BILN | Line notation for biopolymers | `molecule/idt_alias.h`, KET aliases |
| CIP | Cahn–Ingold–Prelog stereo descriptors (R/S, E/Z) | `molecule/molecule_cip_calculator.h` |
| CMF | Indigo's compact binary molecule format, used as the stored form in Bingo | `molecule/cmf_*.h` |
| CTfile | The BIOVIA family of formats: MOL, SDF, RXN, RDF | loaders/savers, `documentation/` |
| Deconvolution | Splitting a set of structures into a scaffold plus R-group substituents | `indigo-deco`, R-group code |
| Fingerprint | Bit vector summarising substructures, used as a search pre-filter | `molecule/molecule_fingerprint.h` |
| HELM | Notation for macromolecules (peptides, nucleotides) | KET / monomer code |
| InChI | IUPAC canonical identifier, produced by the bundled third-party library | `molecule/inchi_*.h` |
| KET | Ketcher's JSON document format; the interchange format between Indigo and Ketcher | `molecule/ket_*.h` |
| Markush structure | A generic structure with R-group substitution sites | R-group code |
| Monomer | A macromolecule building block (amino acid, nucleotide, CHEM) | KET / monomer library |
| Query molecule | A structure whose atoms/bonds carry predicates instead of concrete values | `QueryMolecule` |
| RGroup / SGroup | Substitution site set / labelled atom set (superatom, SRU, data group) | `BaseMolecule` |
| SMARTS | Substructure query language | `molecule/smiles_loader.h` (query mode) |
| SMILES | Line notation for structures; "canonical SMILES" is the identity form | `molecule/*smiles*` |
| Standardize | Apply a configured set of normalisations to a structure | `molecule/molecule_standardize.h` |
| Superatom | An S-group rendered as an abbreviation (e.g. `Ph`, `Boc`) | S-group code, render2d |
| Tautomer search | Search treating tautomeric forms as equivalent | Bingo search modes |
| Valence model | The rule set mapping element + charge + bonds to allowed valence and implicit H | valence code, loader options |
