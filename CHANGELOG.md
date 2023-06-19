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
* #932  Reagents: When opening Daylight SMILES and Extended SMILES files with reagent the original structure is distorted
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
* #521: core: replace MultiMap in MolfileLoader class by @loimu in #911 
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
