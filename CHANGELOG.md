# Indigo 1.28.0
Released 2025-02-13

## Bugfixes 
* #2313 Indigo functions doesn't work if ambiguous monomer present on the canvas 
* #2435 Export of ambiguous monomers to Sequence/FASTA doesn't work for mixed ambiguous monomers
* #2324 System loads DNA bases instead of RNA ones during IDT import
* #2387 After opening a saved HELM file, microstructure name F1 turns into Mod0
* #2062 HELM loader ignores repeating token
* #2332 Error message should use "ambiguous monomer" instead of "variant monomer" 
* #2338 System loads HELM inline SMILES phosphate as base (RNA1{R[P%91(O)(O)=O.[*:1]%91 |$;;;;_R1$|]}$$$$V2.0)
* #2436 System should report an error if we have one or more monomers on the canvas don't have mapping for them in case of export to Sequence/FASTA
* #2427 Import of unsplit monomers from HELM doesn't work 
* #2041 Monomer could be saved to RXN V3000 format but can't be loaded back - exception
* #2137 An error occurred while saving the unresolved nucleotides and arrow in the RXN3000 format
* #2195 Indigo functions doesn't work if query atom and monomer on the canvas at the same time
* #2126 An error occurred while saving the nucleotide and arrow in the RXN2000 format
* #2330 Error diagnostic is not clear in case of wrong percent value type
* #2622 Saving to MOL 3000 cause template data loss that causes wrong export to HELM
* #2361 Wrong error message if SMILES phosphate has lack of attachemt point 
* #2359 System loads HELM even it it has wrong connection section (PEPTIDE1{[DACys]}|PEPTIDE2{C}$PEPTIDE1,PEPTIDE2,1:R1-1:R2$$$V2.0)
* #2539 Export of unknown monomer to HELM doesn't work
* Revert "#2657  - Add ES6 options for WASM build (#2658)"

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.26.0-rc.1...indigo-1.27.0

# Indigo 1.27.0
Released 2025-02-12

## Features
* #2559 Read Name and Reaction Conditions from RDF Metadata as a text
* #2404 Save Name and Reaction Conditions to RDF Metadata
* #2657 Add ES6 options for WASM build
* #2652 Skip Name and/or Reaction Conditions metadata fields on save to RDF if no data is available
* #2574 Support for Collapsed monomers

## Bugfixes 
indentation errors in docstring description
* #2654 Arrow size become 2 bonds length after indigo changed default arrow size to 1
* #2665 Margins are missing if no reaction result
* #2664 Vertical margin for catalyst is wrong
* #2662 Arrow size is wrong if reaction loaded from SMARTS
* #2653 Indigo should send to ketcher different bond size for macromolecules
* #2647 Reaction's name or conditions text wraps incorrectly in specific cases: the last line before truncating, removing of special symbols, empty line instead of "..."
* #2583 Not correct length of Multi-Tailed and Single arrows for reactions with atoms after loading from RDF or Layout actions 
* #2519 The length of the arrow becomes 2 bonds after layout but should be 1
* #2624 Elliptic arrow rendered wrong while expotring to PNG in remote mode
* #2497 Test fail on compare CML file for R-group

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.26.0-rc.1...indigo-1.27.0

# Indigo 1.26.0
Released 2025-01-10

## Features
* #2460 Load/save hydrogen bonds from/to HELM, KET and MOL
* #2575 Saving atom to monomer connections to Mol and HELM
* #2472 Support for three letter sequences

## Bugfixes
* #2491 Update ketcher to newer version for indigo-service frontend
* #2519 The length of the arrow becomes 2 bonds after layout but should be 1
* #2520 The length of arrow should be 1 bond after importing reaction from rxn file
* #2458 The reaction with catalysts is displayed incorrectly with ACS style setting and after layout
* #2681 Molfile V3000 with SGROUP type DAT fails to load in Ketcher due to missing spaces in FIELDDISP
* #2654 Arrow size become 2 bonds length after indigo changed default arrow size to 1

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.24.0-rc.1...indigo-1.25.0

# Indigo 1.25.0
Released 2024-11-18

## Features
* #2236 Import/export multi-tails in KET
* #2162 Add support for rendering embedded SVGs while exporting to PNG 
* #2129 Render Multi-tail Arrows for Pathway Reactions in PNG and SVG
* #2176 add ACS style reaction layout
* #2209 SVG image support in CDX/CDXML
* #2237 Save multi reactions (pathway, single, edge cases) to RDF
* #2175 Adjust the rendering of PNG and SVG formats for ACS style

## Bugfixes and improvements
* #2293 Export IDT doesn't work for natural ambiguous monomers
* #2383 update lunasvg
* #2309 Update layout for pathway reaction on canvas
* #1989 Convert api does not recognise format during autodetection
* #2070 Settings for the "attachment point tool" don't update with changed pixel settings
* #2457 The reaction is not in the center of the picture after saved in png (svg)
* #2444 The thickness of the arrow is incorrect when saving to png(svg)
* #2447 single up and single down bonds are displayed incorrect when save to PNG(svg)
* #2257 Unable to save CHEM SS3 to IDT format
* #2312 Export of ambiguous monomers doesn't work (error appears) to any export format (even to SVG/PNG) except KET
* #2433 Issues with Saving and Opening Structures with Multi-Tailed Arrow in CDX and CDXML Formats
* #2512 Equilibrium Half Arrows are displayed incorrect when saved to PNG (svg) by default size
* #2217 The wrong direction of the arrows when exporting from CDXML (or CDX, base64 cdx) format and two retrosynthetic arrows
* #2422 Indigo functions doesn't work prorerly: Aromatize, Dearomatize, Layout, Clean Up, Hydrogens, Auto-Mapping Tool
* #2458 The reaction with catalysts is displayed incorrect with ACS style setting and after layout
* #2440 System shouldn't allow user to export alternatives ambiguous monomers to IDT (since only mixtures are supported)
* #2331 System should throw an error in case of wrong IUBcode
* #2122 Sugar R should not save in the IDT format
* #2446 Sub font size is incorrect when save to png (svg)
* #2407 The ordering of branches of cascade reactions and reactions themselves should be the same on Load/Save from/to RDF
* #2478 The arrow is displayed incorrect when import from rxn file
* #2603 Unknown 'a' CIP stereochemistry cause error in CDXML parser


**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.24.0-rc.1...indigo-1.25.0

# Indigo 1.24.0
Released 2024-10-04

## Features
* #2096 Support retrosynthetic arrows - PNG/SVG rendering
* #2097 Add retrosynthetic arrow import/export for cdx/cdxml formats
* #2031 Build a data structure for a reaction tree
* #1619 Import/export of variant monomers from IDT
* #2052 Position pathway reaction on canvas
* #2015 Import/Export of variant monomers from Fasta/Sequence
* #2028 PNG image support in CDX/CDXML
* #2128 Add Multi-tail Arrows to Pathway Reactions Loaded from RDF
* #2188 HELM 2 support: variant monomers
* #2189 HELM 2 support: support for SMILES 

## Bugfixes
* #2232 System replace arrows of diffrent types with Open Arrow type while loading from PPTX(CDX)
* #2249 Arrow Open Angle changes to Arrow Filled Triangle after saving and opening in CDX/CDXML
* #2120 Export the structure for nucleotides 2-Amino-dA does not work correctly
* #2266 Error message is wrong for missing ratio number (PEPTIDE1{(A:+C:0.1)}$$$$V2.0)
* #2300 HELM loading doesn't work in remote version
* #2293 Export IDT doesn't work for natural ambiguous monomers
* #2336 Ribose sugar doesn't allow to load IDT custom mixed bases
* #2358 Import of monomer with only one connection point (R2) doesn't work
* #2321 Export of multi-character monomer IDs inside ambiguous monomer works wrong
* #2375 Export to IDT doesn't work at all for mixed bases
* #2356 Library ambiguous peptides loaded as mixtures from FASTA
* #2357 Export to HELM doesn't work if we connect peptide TO molecule
* #2303 Import of variant monomers from Sequence doesn't work for RNA and FASTA for RNA/DNA
* #2355 Import of HELM with fractional ratio mixture values doesn't work - system expects integer
* #2421 Incorrect stereo-label placement for (E) and (Z) (indigo part)
* #2417 The layout is incorrect with retrosynthetic arrow

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.24.0...indigo-1.23.0

# Indigo 1.23.0
Released 2024-09-06

## Features
* #2134 Store monomer library as API object 
* #2144 Import/export images in KET 
* #2071 add retrosynthetic arrow 
* #2034 Introduce variant monomers in a model 
* #2030 Load RDF Format as simple list of reaction 

## Bugfixes
* #1944 Fullerene and 3D molecules are not represented as 3D in the Miew 3D window 
* #2213 Layout for 1-step reactions is broken  

## Improvements
* #2131 Create Indigo WASM bundle with removed rendering part  
* #2139 use \<filesystem\> 
* #2141 use \<chrono\> instead of nano 

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.23.0...indigo-1.22.0

# Indigo 1.22.0
Released 2024-09-06

## Features
* #1912 Support unresolved IDT monomers (no structure, IDT only) 
* #1919 Support of unsplit nucleotides 
* #1188 HELM ver 1 scope: support multiple sequences and connection tables (import and export) 
* #1188 HELM ver 2.04 support API for ketcher

## Bugfixes
* #1993 Micro and macro structures connected through attachment points cannot be opened after save in CDXML format in micro mode 
* #1986 System breaks IDT export line if it is longer than 80 chars by
* #2106 Export monomer with R2, R3, R4 APs resulted in monomer with R2, R3 if exported to mol v3000
* #2110 The structure changes when saved in SVG/PNG formats 

## Improvements
* #877 api: wasm: indigo-ketcher: prepare 2-files wasm+js package 

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.22.0...indigo-1.21.0

# Indigo 1.21.0
Released 2024-08-27

## Features
* #1903 Introduce IDT alias for monomers and RNA presets in Ket format
* #1901 Receive monomer library from Ketcher upon import/export IDT notation
* #1900 Export of modified RNA to IDT notation (modified IDT monomers)
* #1899 Import of modified IDT monomers

## Bugsfixes
* #1974 Preview: User can save IDT with 5' phosphate but cannot open this file again.
* #1972 Can't save KET with nameless superatom
* #1996 Cannot open IDT with MOE sugar.
* #1960 Fix error message in the SequenceLoader for an invalid sequence
* #1928 Support monomer to molecule connections type
* #1878 The structure saved in CML format does not open
* #1843 FASTA export: 80 chars limit
* #2000 Exporting a CDX file to ChemDraw loses information about charge on atoms
* #1997 Export to IDT doesn't work at all for remote indigo
* #1993 Micro and macro structures connected through attachment points cannot be opened after save in CDXML format in micro mode
* #2004 Errors occur when trying to save a macro structure connected to a micro structure to MOL V3000
* #1994 Micro and macro structures connected through attachment points cannot be opened after save in CDX and Base 64CDX format in micro mode
* #1992 Micro and macro structures connected through attachment points cannot be saved in Extended SMILES format in micro mode
* #1990 Micro and macro structures connected through attachment points cannot be saved in CML format in micro mode
* #1984 Error message is wrong if in case if position indicator in IDT code contradicts real position of the monomer in the chain
* #1982 Cannot open some expected IDT.

## Improvements
* #1976 Too slow monomer library load

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.21.0...indigo-1.20.0

# Indigo 1.20.0
Released 2024-07-01

## Features
* #1654 Export of RNA presets to IDT notation (standard IDT monomers)
* #1588 Import of standard IDT monomers
* #1466 Export as sequence string for RNA, DNA and PEPTIDEs


## Bugs
* #1891 Bug Report | When building api/http container, it cannot run due to mismatch in pydantic version
* #1598 Macro: V3000 export: leaving groups are displayed as side chain connections for standard presets added to canvas 
* #1579 Unable to add hydrogens for aromatic bonds outside the ring - system throws exception
* #1910 Indigo incorrectly calculates RNA coordinates with data from FASTA/Sequence file
* #1950 Empty headers appear when exporting to FASTA

## Improvements
* #1802 bad performance during the parsing of large sequences

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.19.0...indigo-1.12.0

# Indigo 1.19.0
Released 2024-05-02

## Features
* #1755 Import and export fasta format for RNA, DNA and PEPTIDES
* #1466 Export as sequence string for RNA, DNA and PEPTIDEs

## Bugs
* #1787 Some Bonds from ChemDraw don't open in the Ketcher
* #1776 Cannot add to Canvas saved in the Ketcher CDX file with Bond structures
* #1775 Save to CDX converts triple bond to Single/Double Aromatic bond
* #1774 Save to CDX converts aromatic benzene ring to dashed ring
* #1425 Indexing of a substantial quantity of molecules instigates the indexing process for the entire file.
* #1575 Basic hydrogen layout problem - angles of hydrogens are not perfect
* #1771 Bingo version does not return current tag
* #1761 Can't export sequence with 1 monomer to molv3000
* #1598 Macro: V3000 export: leaving groups are displayed as side chain connections for standard presets added to canvas
* #1477 sequence id calculation for molv3000
* #1563 Unable to load mol file: System throws exception: Convert error! Unexpected token '<', "<!DOCTYPE "... is not valid JSON
* #1852 Macro(Export FASTA): The header 'Sequence N' is missing, and '>' symbol is located at end of the first sequence
* #1822 Fasta: All Peptides should be saved to FASTA format and (use N/X symbol as well).
* #1851 Macro(Import FASTA): The ';' symbol is not recognized as a comment
* #1786 Some bonds from Ketcher don't display correctly in the Ketcher and the ChemDraw for CDXML format. 
* #1777 Some bonds don't display correctly in ChemDraw when opening a saved Ketcher CDX file.
* #1849 Macro(Import FASTA): The ' * ' symbol occurring between two letters is not recognized as a break in peptide chain
* #1850 Macro(Import FASTA): The '>' symbol is not recognized as indicating a new sequence
* #1802 bad performance in WASM during the parsing of large sequences
* #1848 Cannot save FASTA using the Remote mode
* #1791 Aromatic is lost from Benzene while save to CDX/base64 CDX formats
* #1774 Save to CDX converts aromatic benzene ring to dashed ring
* #1773 Save to CDX converts plus and arrow into lone pair and line
* #1796 Some bonds from ChemDraw don't display correctly in the Ketcher.
* #1814 Macro: System ignore spaces and line breaks when importing a sequence through Paste from Clipboard
* #1801 No monomer full name when import from a sequence
* #1161 bingo unable to index on sql server
* #1773 Save to CDX converts plus and arrow into lone pair and line
* #1774 Save to CDX converts aromatic benzene ring to dashed ring 
* #1796 Some bonds from ChemDraw don't display correctly in the Ketcher. 
* #1881 Macro: Cannot load Peptides from our Library that are not connected by bonds using FASTA file 

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.18.0...indigo-1.19.0

# Indigo 1.18.0
Released 2024-02-29

## Features
* #1412 Nucleotide splitting for V3000 molfile SCSR  
* #1450 No naturalAnalogShort in KET for basic aminoacids during conversion MOLV3000 -> KET 
* #1436 Expose Fold/Unfold hydrogens function in indigo API
* #1440 Add support for query features in MOL, SDF and RXN formats (Marvin extension)
* #1426 Import sequence format for RNA, DNA and PEPTIDES 
* #1589 Apply hydrogens folding/unfolding with respect to selected atoms 
## Bugs
* #1307 Error in DevTool console about memory access out of bound when call 'ketcher.getRxn()' 
* #1431 Crash during parsing query mol-file
* #1439 Indigo can't parse KET with R-Site as a leaving group 
* #1232 Multi-line reaction cause access violationg exception.
* #1423 Common atoms loaded as aliphatic in SMARTS mode
* Update bug_report.md
* #1458 Failed UT api\indigo_test.py:test_convert_smarts 
* #1460 ImplicitH set to zero casue error loading query molecule from ket 
* #1446 Dearomatization does not work with query features
* PostgreSQL 11 EOL support 
* #1465 Unable to load specific mol-file
* #1484 Query molecule convert implicit/explicit hydrogens cause error 
* #1512 Broken string-formats support for wasm 
* #1452 Convert from implicit hydrogens change layout 
* #1463 Macro: Some molecules are not perfect on preview tooltip 
* #1524 Dearomatizing doesn't work for molecula with custom query fetures 
* #1568 Wrong molecule nodes enumeration in KET-file 
* #1476 Aromatization/Dearomatization wipes out SOME Ring bond count values 
* #1478 Dearomatization causes exception in case of Implicit H count query feature set to 4 (i.e. more than 2) 
* #1525 System attach two explicit hydrogens to aromatized ring 
* #1573 Add/Remove explicit hydrogens can't be applied to fullerene C60 - system throws exception 
* #1567 System can't copy atom with custom query feature to clipboard 
* #1534 Presence of stand alone H2 molecule on the canvas breaks Add explicit hydrogens feature (it stops working) 
* #1468 Valence lost on loading molfile with MRV extension.
* #1472 Dearomatization wipes out Aromaticity query property
* #1504 Molfile MRV extension generated for "H count"
* #1500 Atom Query feature export: System replace "Ring membership" values with "Ring Bond Count" ones for value 0 - export to SDF V2000 file 
* #1564 Add/Remove explicit hydrogens wanishes "Chirality" value 
* #1598 Macro: V3000 export: leaving groups are displayed as side chain connections for standard presets added to canvas 
* #1533 System attach two explicit hydrogens to atoms connected to "any type" bonds 
* #1608 convert_explicit_hydrogens response doesn't comply with the ket schema 
* #1538 Add/Remove explicit hydrogens feature doesn't work if atom with problem valence present on canvas (crash happens) 
* #1614 Adding hydrogens doesn't work for bonds with No center value of Reacting Center 
* #1550 Add/Remove explicit hydrogens feature doesn't work for "Any Atom" molecule with valence value set
* #1593 Macro: V3000 import: removed 5' phosphate is displayed in Ketcher
* #1483 Unable to past empty reaction (arrow only) from clipboard to canvas
* #1607 Unable to load large base64 CDX content to canvas if remote indigo used`
* #1536 Add/Remove explicit hydrogens feature doesn't work for reactions on canvas (crash happens)
* #1652 Unfold hydrogens does not select added bonds 
* #1640 System adds hydrogens for only one atom among many selected by 
* #1634 Add/Remove hydrogens doesn't work for atoms with Radical=Triplete if atom with query feature present on the canvas
* #1629 Add/Rmove hydrogens process should count R-Group attachment point as hydrogen
* #1695 CDX loader failed if object with zero id follow by reaction 
* #1697 CDX loader crashed on some files with abbreviations by 
* #1684 System put on the canvas two arrows (one above other) per each arrow on target CDX 
* #1724 Fold hydrogen in reaction with selection works wrong by 
* #1730 UT cano/permutations is too slow by 
* #1576 Bad molecule layout after adding hydrogens (Chlorophyll A) 
* #1685 System shows attached abbreviation group in wrong position 

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.16.0...indigo-1.18.0

# Indigo 1.16.0
Released 2024-01-30

## Features
* #1185 Macromolecules export from MolV3000 to KET
* #1381 Macromolecules import from KET

## Bugs
* #1395 Class and naturalAnalog of KET monomerTemplate should be optional
* #1405 Valence is lost for Mol and Rxn files
* #1415 Incorrect S-Group count in MOL V3000 header if ImplicitH is set
* #1346 Extended SMILES: Atropisomer is displayed incorrectly
* #1304 Indigo accepts `[#6]` notation in SMILES mode
* #1355 Error appears when pressing 'Layout'
* #1330 Implicit H count is not added to the SMARTS file
* #1358 Error while loading `[!#6,!#7,!#8]` smarts
* #1357 Wrong atom list when paste structure as SMARTS
* #1292 valid SMARTS with cycles cause error in loader
* #1349 Symbol for topology chain is missing in SMARTS file
* #1351 Error returned when try to convert structure with custom query for bond into SMARTS format
* #1283 wrong chirality generated by SMARTS load/save
* #1329 Aromacity is incorrectly marked in the SMARTS file
* #1325 Indigo should return warning for some attributes
* #1328 Chirality is not added to the SMARTS file
* #1281 Support SMARTS "or unspecified" bond property in custom queries
* #1321 When saving a structure with set up Implicit H count and any other atom attribute then an error appears
* #1309 Error appears while trying to save some element with set up H count and implicit H count
* #1319 Directional bonds are encoded incorrectly in SMARTS
* #1183 Support monomer templates import from KET-format
* #1184 Support monomer templates export into KET-format
* #1310 Error appears while opening SMARTS file with query which contains comma
* #1322 Reaction cannon be saved in convert function
* #1303 Return original format from convert and layout function
* #1316 SMARTS saver miss component level grouping
* #1254 SMARTS with component-level grouping saved without '()'
* #1252 SMARTS loader load grouped components as separate molecules
* #1245 Reaction/Molecule autoloaders don't load SMARTS
* #914 Why is the code InChI=123 valid?
* #1224 Different indent after loading reaction from file and after layout it.
* #1221 An empty structure is returned given incorrect InChi string

## Improvements
* #1338 Use docker image for building npm packages and apply tags while publish

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.14.0...indigo-1.16.0

# Indigo 1.14.0
Released 2023-11-07

## Features
* #1217 Support SDF-format for Indigo-Ketcher API
* #1257 Enhanced stereo labels on atropisomers are lost when opening saved Extended SMILES

## Bugs
* #887 When saving in PNG and SVG format, files have low resolution when opened (Ketcher Remote)
* #1200 MolfileSaver add Data S-Groups to molecule at saveMolecule()
* #1199 CanonicalSmiles saver contains extended part.
* #1161 Bingo unable to index on SQL Server
* #1040 SMILES/SMARTS import: Files with S-Group Properties cause an error 
* #1145 CDX import: letter is duplicate and 'superscript' and 'subscript' become the same size as the letter
* #1191 Bingo-elastic: Cannot search at elastic using custom-index
* #731 Warnings in the Structure Check window doesn't display for molecules with R-Groups
* #1230 Unable to save file if the canvas has a reaction arrow and a Functional Group
* #1249 Incorrect order of aliases in SMILES saver
* #1240 Unable to open the CDX file with an R-Group added to the whole structure
* #1239 CDX: An abbreviation appears in the upper left corner of a Function Group or Salt when opening a saved file
* #1287 New S-Group type Query component level grouping
* #1260 SMARTS saver works incorrectly for non-query entities if atom valence or radical present
* #1300 Stereobond is not preserved after pasting a SMILES structure
* #1277 Atropisomer centers support
* #1347 When pasting Extended SMILES with coordinates enhanced stereochemistry is lost

## Improvements
* #1037 Indigo Toolkit information in the 'About' section that is not relevant for the users
* #1197 Migrate indigo-knime nodes to KNIME Analytics Platform 5.1

 **Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.13.0...indigo-1.14.0

# Indigo 1.13.0
Released 2023-10-04

## What's Changed

## Bugfixes
* #1205 Reagent located at the bottom of the arrow when opening the RXN V2000 and V3000 files are located on top of the arrow
* #1168 Error message when trying to save structure with Multiple Group type applied to entire structure
* #1166 CDX: file with R-Group label saved in Ketcher opens without part of structure
* #1159 [CDX] IndigoException: stoi when reading USPTO CDX file
* #1155 [CDX] Indigo header files doesn't appear in msvc solution.
* #1152 No module named tzdata while running indigo service
* #1139 core dumped when reading CDX file downloaded from USPTO
* #1113 RXN 3000 import: When importing, the structure becomes unreadable
* #1094 Structure with R-Group isn't opened correctly from v3000 mol file
* #1061 [Bingo-Elastic] Cannot create custom index in python bingo-elastic
* #1026 [Bingo-Elastic] SVG/PNG: Contracted 'Functional Groups' and 'Salts and Solvents' are rendered expanded when saved
* #926 CDXML import: 'superscript' and 'subscript' appears below the letter

## Features
* #1182 Enhanced stereo labels on atropisomers are lost when opening molfiles
* #1158 Ketcher needs to correctly serialize/deserialize attachment point information for super atoms for mol v3000 & ket format

## Improvements
* #1111 api: add method for copying RGroups for Java and .NET
* #1124 SMILES format does not store alias information

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.12.0...indigo-1.13.0

# Indigo 1.12.0
Released 2023-07-09

## What's Changed

## Bugfixes
* #965 MDL Molfile v3000: when opening files containing 'Salts an Solvents', names are truncated and abbreviation is expanded
* #1036 SMILES import: general chiral specification labels (TH, AL, SP, TB, OH ) don't work
* #1051 Opening file with a superatom label saved in RXN v3000 format only the first part of the label is displayed
* #1114 Atoms of Benzene ring become Monoradicals when opened from file saved in Daylight SMARTS
* #1132 SMILES loader uninitialized heap fix
* #1102 When pasting Extended SMILES structure with stereochemistry there are two &1 centers instead of an ABS and an &1
* #1135 C library macro - va_end() is missing before return statement.
* #1126 Segfault when iterating CDX file from USPTO downloads
* #1144 Unable to save the structure after clicking 'Save', an error appears

## Improvements
* #1098 api: add method for copying RGroups

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.11.0...indigo-1.12.0

# Indigo 1.11.0
Released 2023-06-19

## What's Changed

## Features
* #1053 Split publish job in "Indigo CI" GitHub Action
* #310 Support stereo CIP calculation in Ket format
* #957 Support of Korean, Chinese and Japanese characters in Standalone.
* #995 Automated memory leaks testing

## Bugfixes
* #1044  SVG/PNG: Reaction arrows are not visible without structures at preview and in saved files
* #932 Reagents: When opening Daylight SMILES and Extended SMILES files with reagent the original structure is distorted
* #1084 Can't open mol v3000 files with 'S-Group Properties Type = Generic' and 'S-Group Properties Type = Multiple'
* #1083  Indigo Service: enable of using Indigo Options
* #910  MDL Molfile v3000 encoding: Automatic selection of MDL Molfile v3000 encoding doesn't work if the number of atoms (or bonds) exceeds 999
* #956  Copy Image: When inversion type is chosen in the atom's properties, it is not saved
* #955 Copy Image: Saved bonds does not have Reacting Center marks
* #1052 Set "Indigo Docker images preparation" GItHub Action to start manually only add version tag to Docker images
* #1064 Keep implicit hydrogens information in KET-format
* #1048 Memory leak in 3rd party library
* #1056 RXN2000/3000 should not serialize INDIGO_DESC fields for s-groups
* #1050 Memory leak in StringPool code
* #1031 Calculate CIP: Hovering over the label R/S displays Indigo system information
* #1049 Memory leak in the SMILES loader code
* #973 Daylight SMARTS: Error when save file in SMART format with reaction arrow and reagent
* #1017 imagoVersions is undefined
* #899 Add restrictions on size to be less than 1000
* #1015 Cannot test CDX export with certain files
* #944 CDX import: Greek letters, Celsius and Fahrenheit signs are replaced with question marks
* #1093 python binding memory leak from 1.8.0 (and still present in 1.10.0)

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.10.0...indigo-1.11.0

# Indigo 1.10.0
Released 2023-03-22

## What's Changed

## Features
* #941 CDX export

## Bugfixes
* #1003 Some texts are not rendered and may lead to Indigo crash
* #987 docker-indigo-tester image build failed
* #994 Some UTF-8 characters from Ketcher Text panel are not displayed in Indigo WASM
* #889 When saving in PNG and SVG format UTF-8 text display incorrectly (Ketcher Standalone)
* #1032 Combine molecules that are related to a single s-group into one in .Ket format
* #974 SVG/PNG: Molecule reagent located below arrow is displayed in preview above arrow
* #1039 Opening file with a superatom label saved in RXN v3000 format removes a custom s-group
* #1063 Structure saved in CDX and Base64CDX with reaction arrow cannot be opened 
* #1068 CDX-loader crash 

**Full Changelog**:https://github.com/epam/Indigo/compare/release/1.9...release/1.10

# Indigo 1.9.0
Released 2023-01-31

* MDL Molfile v3000 encoding: Automatic selection of MDL Molfile v3000 encoding doesn't work if the structure contains Enhanced stereochemistry by @mkviatkovskii in #924
* cdx import in scope of current KET/CDXML features support by @even1024 in #885
* Structures with the arrow lose their integrity when pressing 'Layout' by @even1024 in #938
* Abbreviations are not supported by @even1024 in #685
* #934: api: tests: IronPython update to 3.4.0, fix tests by @mkviatkovskii in #940
* Add support of R-groups to the CDX loader. #36 by @even1024 in #946
* CDX import: Reaction arrows disappear when opening a file #943 by @even1024 in #948
* CDX import: Aromatized structures are not recognized when Pasting from Clipboard #950 by @even1024 in #953
* CDXML parser memory leak #966 by @even1024 in #967
* Error opening MOL and RXN files with RBC/SUB/UNC queries #928 by @even1024 in #969
* CDX Import, CDXML Import: parsing error when superatom starts with 'R' symbol #960 by @even1024 in #975
* CDXML: When opening a saved file with text, the Font size enlarges #961 by @even1024 in #982
* CDXML: When opening a file saved with 'Any atom', 'Atom Generics' or 'Group Generics' structure loses its integrity #968 by @even1024 in #985
* CDXML import fails to load rectangle primitives #979 by @even1024 in #980
* CDXML: File containing Functional Groups or Salts and Solvents cannot be opened and causes a convert error #963 by @even1024 in #984
* CDXML import: nodes with radicals are not getting parsed #990 by @even1024 in #991
* CDXML import: fails to import some cdxml files with multiple text objects related to different fragments by @even1024 in #993
* CDXML import: 'superscript' and 'subscript' is not displayed correctly #962
* Improve ssl bingo elastic by @MysterionRise in #901
* bingo: postgres: add support for Postgres 15, drop support for Postgres 10 by @mkviatkovskii in #903
* #521: core: replace MultiMap in MoleculeRGroupsComposition class by @loimu in #917
* #521: core: replace MultiMap in MolfileLoader class by @loimu in #911�
* #929: fix auto-saving to CTAB v3000 by @mkviatkovskii in #931

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.8.0...indigo-1.9.0

# Indigo 1.8.2
Released 2022-11-28

## What's Changed
* core: SMARTS support for implicit hydrogens specifier 'h' added by @mkviatkovskii
* Feature/#861 cdxml enhanced stereochemistry by @even1024
* Feature/#862 cdxml abbreviations by @even1024
* Bugfix/#891 dearomatize query onload by @even1024
* Bugfix/#870 and bugfix/#871 multistep to rxn-smiles by @even1024

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.8.0...indigo-1.8.2

# Indigo 1.8.0
Released 2022-10-14

## What's Changed
* ci: fix npm versioning according to semver by @mkviatkovskii in https://github.com/epam/Indigo/pull/741
* Fixed mistypes by @SunFellow in https://github.com/epam/Indigo/pull/743
* Rename `indigo-ketcher.zip` artifact to `indigo-wasm.zip` by @SunFellow in https://github.com/epam/Indigo/pull/744
* api: added TPSA calculation by @mkviatkovskii in https://github.com/epam/Indigo/pull/745
* api: added numRotatableBonds, numHydrogenBondAcceptors, numHydrogenBondDonors by @mkviatkovskii in https://github.com/epam/Indigo/pull/748
* api: added logP and molarRefractivity, removed Python implementation by @mkviatkovskii in https://github.com/epam/Indigo/pull/750
* Text cipfix by @even1024 in https://github.com/epam/Indigo/pull/739
* api: added atom hybridization calculation by @mkviatkovskii in https://github.com/epam/Indigo/pull/751
* api: renamed cLogP to logP by @mkviatkovskii in https://github.com/epam/Indigo/pull/752
* api: bingo-nosql: fix partitioning in exact search by @mkviatkovskii in https://github.com/epam/Indigo/pull/754
* api: c: another stage of abandoning self-written red-black tree collections by @mkviatkovskii in https://github.com/epam/Indigo/pull/756
* api: cpp: replace multiple Indigo*Iterator classes to single template one; add Bingo.Part test by @mkviatkovskii in https://github.com/epam/Indigo/pull/757
* metadata refactoring by @even1024 in https://github.com/epam/Indigo/pull/755
* core: ket: text rendering support by @even1024 in https://github.com/epam/Indigo/pull/760
* core: molecule auto loader: cdx enable; text for reaction fix (#706) by @even1024 in https://github.com/epam/Indigo/pull/763
* core: base64 support for binary formats, uninitialized data and #764 fix by @even1024 in https://github.com/epam/Indigo/pull/766
* #768: api: add hash() method for molecules and reactions, improve reaction hash to decrease number of collisions by @mkviatkovskii in https://github.com/epam/Indigo/pull/769
* Bingo Elastic search by @AATDev21 in https://github.com/epam/Indigo/pull/772
* #770: reduce list of exported symbols to avoid conflicts with other shared libraries by @mkviatkovskii in https://github.com/epam/Indigo/pull/793
* #791: core: molecule: Lee-Crippen SMARTS pKa calculation method implemented by @mkviatkovskii in https://github.com/epam/Indigo/pull/792
* #794: ci: add version changing script and update version to 1.8.0-dev by @mkviatkovskii in https://github.com/epam/Indigo/pull/795
* core: render2d: text renderer fixes, UTF-8 support, subscript/superscript, all arrows styles, fix for #355 by @even1024 in https://github.com/epam/Indigo/pull/776
* #777: utils: indigo-service: reorganize codebase, simplify docker compose, update ui by @mkviatkovskii in https://github.com/epam/Indigo/pull/796
* #798: api: python: reorganize codebase by @mkviatkovskii in https://github.com/epam/Indigo/pull/799
* #163: api: added test by @mkviatkovskii in https://github.com/epam/Indigo/pull/802
* Issue #520: api: replace RedBlackMap in IndigoDeconvolution by @AStepaniuk in https://github.com/epam/Indigo/pull/803
* #808: render2d: equilibrium arrows rendering supported by @even1024 in https://github.com/epam/Indigo/pull/801
* Fix for Indigo-WASM build by @SPKorhonen in https://github.com/epam/Indigo/pull/828
* Restore indigo service by @AlexanderSavelyev in https://github.com/epam/Indigo/pull/858
* Issue #520: core: replace RedBlackMap and RedBlackSet implementation by @AStepaniuk in https://github.com/epam/Indigo/pull/804
* Issue #520: core: replace RedBlackMap for GraphEmbeddingsStorage by @AStepaniuk in https://github.com/epam/Indigo/pull/805
* Issue #520: core: replace RedBlackMap in SimpleCycleBasis by @AStepaniuk in https://github.com/epam/Indigo/pull/806
* Issue #520: core: replace RedBlackMap for MangoPg by @AStepaniuk in https://github.com/epam/Indigo/pull/809
* Issue #520: core: replace RedBlackMap in MoleculeRGroupsComposition class by @loimu in https://github.com/epam/Indigo/pull/810
* Issue #520: core: replace RedBlackMap in CanonicalSmilesSaver by @AStepaniuk in https://github.com/epam/Indigo/pull/811
* Issue #520: core: replace RedBlackMap in BaseMolecule class by @loimu in https://github.com/epam/Indigo/pull/812
* Issue #520: core: replace RedBlackMap for Molecule3dConstraints by @AStepaniuk in https://github.com/epam/Indigo/pull/813
* Issue #520: core: replace RedBlackMap in MoleculeGrossFormula class by @loimu in https://github.com/epam/Indigo/pull/814
* Issue #520: core: replace RedBlackMap in MultipleCdxLoader class by @loimu in https://github.com/epam/Indigo/pull/816
* Issue #520: core: replace RedBlackMap  in MolfileSaver class by @loimu in https://github.com/epam/Indigo/pull/819
* Issue #520: core: replace RedBlackMap in BaseReactionSubstructureMatcher class by @loimu in https://github.com/epam/Indigo/pull/820
* #843 Make core List moveable by @AStepaniuk in https://github.com/epam/Indigo/pull/844
* Fix indigo-service for config not found message issue by @AlexanderSavelyev in https://github.com/epam/Indigo/pull/873
* CDXML agents and reactions support. #832, #836, #837, #835, #834, #832, #830, #853 by @even1024 in https://github.com/epam/Indigo/pull/872
* S-Groups support for extended SMILES (#874) by @even1024 in https://github.com/epam/Indigo/pull/875

## New Contributors
* @AStepaniuk made their first contribution in https://github.com/epam/Indigo/pull/803
* @loimu made their first contribution in https://github.com/epam/Indigo/pull/810

**Full Changelog**: https://github.com/epam/Indigo/compare/indigo-1.7.0...indigo-1.8.0-rc.1

# Indigo 1.7.0
Released 2022-05-26

## Features
* API web service: added /render endpoint for rendering compounds and reactions
* API: added logP calculation to Python API
* API: added atom hybridization calculation to Python API
* API: added salt stripping method to Python API
* Core: added support for multistep reactions
* Ketcher WASM API: added InChIKey calculation method
* API: Added Jupyter notebooks with examples of using Indigo in machine learning
* API: Added initial version of graph neural networks featurizers

## Improvements
* ZLib updated to 1.2.12
* LibPNG updated to 1.6.37
* TinyXML updated to TinyXML2 9.0.0
* Bingo PostgreSQL support to Postgres 13 and 14 added, thanks @SPKorhonen;
  dropped support for Postgres 9.6

## Bugfixes
* Bingo Elastic: fixed exact search (#644)
* Core: Ketcher format loader: options handling fixed (#588)
* API: Fixed `name()` calling for RXNV3000 format (#678)
* Numerous fixes for Ketcher data format (#689, #711, #733, #734)
* API web service: fixed descriptors calculation


# Indigo 1.6.1
Released 2021-12-28

## Features
* PoC implementation of Indigo modern C++ user API written on top of low-level C API. Later it will be used
  in Indigo-WASM and probably other languages.
* New Indigo service added as preview. Modernized Indigo service implements JSON:API protocol and can be installed as 
  Docker image `epmlsop/indigo-service:enhanced-latest`.
* Indigo API ported to ARM64 processor architecture. Python, Java and C# wrappers now contain required native libraries 
  for macOS (Apple M1) and Linux.
* Implemented loader for CDXML format.
* Dative and hydrogen bonds are now supported.
* Implemented partial aromatization/dearomatization for the structures with superatoms.
* Multifragment support for KET-format.
* Simple objects support for KET-format.
* Atom's aliases and functional groups' attributes support for KET-format.
* Indigo-Python: initial version of inorganic salt checker added.

## Improvements
* Bingo-NoSQL major refactoring with significant multithreading performance improvements.
* C++ unittests were separated in API and Core parts.
* CMake build system by default tries to enable as many components as possible and warns if building something
  is not possible on the current platform.
* Migrated to modern C++ standard mutexes and locks instead of own-written implementation.
* Using thread-safe objects in Indigo API instead of raw mutexes to guarantee thread safety.
* C++ code modernization: added 'override', replaced plain C functions with corresponding from std, etc.
* Indigo API integration tests engine parallelized. 
* Indigo WASM API for Ketcher reached stable status and is now published to NPM public repository.
* Indigo i386 libraries for Windows prepared.
* CI/CD: automatic code style checks and linters added for Python and C++ code.

## Bugfixes
* Fixed multiple data races in API and especially in Bingo-NoSQL (#476).
* InChI library bugfix for empty string support
* Multiple small bugfixes in Indigo-Ketcher WASM module and Indigo Service.
* Bingo-Elastic-Java: updated all dependencies to fix log4j security issue.
* Fixed an occasional error in RPE.
* Bingo-NoSQL: fixed `enumerateId()` in Java.


# Indigo 1.5.0
Released 2021-09-06.

## Features
* InChI updated to 1.06
* Added WebAssembly support to run Indigo in a web browser. 
* Added Java and Python API for Bingo Elasticsearch cartridge
* Added JSON-based data format for interacting with Ketcher supporting enhanced stereochemistry, simple graphics,
  reactions and r-groups.

## Improvements
* Added C++ unittests.
* Added multiple API integration tests.
* CMake build system reworked, now all components, including Bingo cartridges and Python, Java, C# API.
  could be built using single CMake command.
* Migrated to standard modern C++ smart pointers. Changed AutoPtr to std::unique_ptr.
* Unified molecule check function and changed the result format.
* Miscellaneous modern C++ related refactorings: added 'override', replaced plain C functions with corresponding from std.
* Optimized the adding of elements to atoms and bonds arrays.
* Exposed oneBitsList in .NET API.
* Implemented context manager and iterator for Bingo object.
* Added Bingo CI for Postgres 9.6.

## Bugfixes
* api: sessions fixed, now options are session-scoped, not global.


Previous versions: 
* [Indigo API](/api/CHANGELOG.md)
* [Bingo](/bingo/CHANGELOG.md)
