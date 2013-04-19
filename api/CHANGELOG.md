Indigo 1.1.10
----------

*19 April 2013*

This release has two major additions: 

1. We started to support Mono and provide a .NET version that runs on Linux and Mac OS X. This was done with help of developers from Royal Society of Chemistry ([http://www.rsc.org]()) who also helped us to localize and fix several stability issues in our .NET version.

2. Indigo now understand aromatic rings with external double bonds such as O=C1C=CC=CC=C1. To enable this mode you have to set "aromaticity-model" option to "generic" (while default version is "basic").

All changes:

* Mono support: now we provide .NET modules for Windows, Linux, Mac OS X, and a universal bundle that works on all the specified platforms. Thanks to Dimitry Ivanov for various suggestions: [https://groups.google.com/d/msg/indigo-dev/ObLkSecXrOw/g2RGOojqjosJ]()
* Indigo Renderer automatically selects output format based on the file extension. It is not necessary to specify "render-output-format" option if you are rendering into a file.
* Original hydrogens are not folded during molecule transformations. Thanks to Ken for the bug report: [https://groups.google.com/d/msg/indigo-bugs/359gCN9OrMg/fKoMt5kS9XcJ]()
* Either cis/trans bond marked is preserved during molfile loading even if substitutes are collinear. Again thanks to Ken for the suggestion: [https://groups.google.com/d/msg/indigo-bugs/oszcYWrwctI/3t2XuonNAaYJ]()
* Indigo Renderer now has additional options for partial image size specification. These options are: "render-image-width", "render-image-height", "render-image-max-width", "render-image-max-height"
* Fixed an issue in the SMILES loader module that set invalid number of implicit hydrogens: [https://groups.google.com/d/msg/indigo-bugs/i7g3hoSXxhI/XaXsEOVJ8_cJ]()
* Generic aromaticity model that can be enabled by "aromaticity-model" option. Thanks to Daniel for pushing us to implement this functionality.
* Another aromaticity option for find any kekulize configuration if Indigo cannot dearomatize a structure. For example Indigo cannot dearomatize the structure c1ccc2c(ccc2)cc1, because there are no bonds configuration such that the structure is aromatic. But you can try to find approximate kekulize configuration if you specify option "dearomatize-verification" to "false".
* Indigo now uses dearomatization module to find number of hydrogens in aromatic rings in the IndigoObject.canonicalSmiles() method. If hydrogens configuration is ambiguous then Indigo throws an exception about this.
* Additional "unique-dearomatization" option. If this option is set to true Indigo will throw an exception of dearomatization configuration is ambiguous (that means that canonical SMILES cannot be generated): [https://groups.google.com/d/msg/indigo-bugs/WIH8bWQpVHs/Z8VLlXR2U28J]()
* IndigoInchi.loadMolecule can now load molecules from InChI Aux infromation. Thanks to Nico: [http://tech.knime.org/forum/indigo/inchi-conversion-into-2d-structure#comment-28514]()
* Indigo Renderer doesn't have a dependency on the new GLIBC any more. This dependency prevented loading of Indigo Renderer on CentOS 5.9 and less.
* Minor changes in Java and C# bindings: expandAbbreviations method has been add, typo in countDataSGroups has been fixed
* New method to get stereocenter pyramid. Thanks to Daniel for the feature request: [https://groups.google.com/d/msg/indigo-dev/z0a1QwRMrx4/0s-KEtPBB_EJ]()
* Fingerprints computation now works 30% faster.
* All stereocenter exceptions now includes also atom index: [https://groups.google.com/d/msg/indigo-bugs/nZOmFCEsNqk/S92lgH5zjvwJ]()

Indigo 1.1.9
----------

*25 March 2013*

New features:

* Single atoms are encoded into Indigo fingerprint. In the previous versions we enumerated subgraphs starting from 2 atoms.
* new method IndigoObject.resetSymmetricStereocenters to clear stereocenters that are not real stereocenters like in `CC[C@@H](CN)CC`


Changes:

* Implicit hydrogens are rendered better if they are on the bottom of an aromatic ring
* Missing dependency file msvcp100.dll is loaded automatically. 
* Smiles saver doesn't throw exception about implicit hydrogens if they are not saving in SMILES
* Workaround for a clang 3.2 compiler bug that caused incorrect similarity values on Mac OS X platforms

Indigo 1.1.8
----------

*10 March 2013*

New features:

* For superatoms IndigoObject.name() returns superatom label
* New method indigoExpandAbbreviations in the Indigo C interface. Bindings for Java, C# will be ready soon.
* Single record in SDF file is limited to 100Mb to prevent accident out-of-memory exceptions when loading non-SDF files.
* Indigo compact molecule and reaction format is compatible with previous versions. KNIME workflows that are using Indigo nodes will work fine with previously saved workflows.
* Indigo.loadMolecule reads molecule properties from SDF file too. Karen Karapetyan: [https://groups.google.com/d/msg/indigo-general/-2aYCKW8nSs/xteR7sFtsKIJ]()

Fixed:

* Issues with transformations. Thanks to Karen Karapetyan for various bug reports!
* Exception on invalid options in the InChI plugin
* Issues with long multiline in molfile data s-groups
* Aromaticity matcher issue. Thanks to James Davidson for the bug report: [https://groups.google.com/d/topic/indigo-general/yhTfm6XsKTM/discussion]()
* Atom-to-atom mapping timeout issue
* File handlers leak in indigo.iterateSDFile method in case of empty file

Indigo 1.1.7
----------

*24 December 2012*

New features:

* stack usage has been significantly reduced. Almost all the test works find under 256Kb stack limit, meaning that everything should work in .NET and Java environment without any additional settings. Problem appeared in using Indigo in WCF service in IIS. 
* initial implementation of `indigoNormalize` method in Indigo API. It removed hydrogens and neutralize [N+][O-] into N=O. Other transformation are coming soon and suggestions are welcome.
* new similarity methods `normalized-edit` to return an approximate measure of changes that needs to be applied to convert one molecule into another. Used in Imago testing framework to measure recognition quality based on reference files.
* reaction catalysts serialization
* layout method flips a molecule to ensure that that first atom is right to the last one: <https://groups.google.com/d/msg/indigo-general/EBOc2BT1_N0/Gl-2ZpVmUQcJ>
* query molfile saver outputs a number of implicit hydrogens

Fixed:

* substructure matcher throws an exception if molecule has invalid valences: <https://groups.google.com/d/msg/indigo-bugs/IoFmqShx6nE/FinoBUtK-RsJ>
* aromatization method throws an exception if molecule has invalid valences: <https://groups.google.com/d/msg/indigo-general/MlBa6Wc31L8/03i5Yfe0FP4J>
* molecule dearomatization with radicals doesn't work
* several issues in reaction product enumerator
* layout issue: <https://groups.google.com/d/msg/indigo-dev/zWzfGTqMKKw/Fvak2zeYXyoJ>
* another issue with molecule R-groups layout 
* issue with saving a molfile with R-site with index 32 causing high memory consumption. Additional internal check has been added to prohibit unexpectedly large memory allocations (that usually means bug)
* regression in the R-group label method for an R-site without any number
* bug in the highlightedTarget method if a molecule has been changed before
* SVG multithreaded rending has been disabled due to the potential issue with Cairo libraries. Need to update Cairo libraries to check if problem still appear.
* issue with tautomer substructure matching for the aromatic compounds
* molecule aromatization method doesn't affect R-group fragments

Indigo 1.1.6
----------

*15 October 2012 (no public announcement)*

* option to preserve atom and bond ordering during serialization process. Used in KNIME: <http://tech.knime.org/forum/indigo/indigo-bug-in-handling-sdfs>
* reaction product enumerator handles larger class of transformations
* option `smarts` for indigo-depict to draw SMARTS

Indigo 1.1.5
----------

*28 September 2012*

New features:

* Rutherfordium isotopes atomic weights added
* Additional check for an invalid stereocenter when an angle between bonds is small.  
Thanks to Karen: <https://groups.google.com/d/msg/indigo-general/zYHfZs9V72Q/lB_KHfMxmi8J>
* Options passed to the InChI plugin are being automatically corrected independent of OS. You can use both '-' and '/' prefixes on all the systems.  
Look more at <http://ggasoftware.com/opensource/indigo/api#options>  
Thanks to Karen for the suggestion: <https://groups.google.com/d/msg/indigo-general/XxPC2EuT0-g/7kNUasWWpz0J>
* Build scripts now work with Visual Studio 2012
* Cross-like layout of atom with four bonds for molecules like CCS(=O)(=O)CC

Fixed:

* Isotope values in the InChI -> Molecule conversion method are being shifted by 10000
* Issues in the Reaction Product Enumerator and the Transformation algorithm causing less correct results to appear
* Exceptions during Molecule -> InChI conversion on Mac OS X if a molecule is passed in an aromatic form that cannot be dearomatized

Indigo 1.1.4
----------

*13 September 2012*

* Allene stereocenters detection algorithm is not throwing exception in case there are not atom coordinates
* Aromatic [si] can be loaded from SMILES
* Dearomatization improvements for B, Si, P atoms. There was an issue that the dearomatization method didn't work with the molecule CB1OB(C)OB(C)O1 if it was loaded from SMILES in aromatic form.

New methods:

* setExplicitValence.
* radical, setRadical. This methods returns and accepts Indigo.SINGLET, Indigo.DOUBLET or Indigo.TRIPLET radicals.

Thanks to Ferenc for suggestions to add these methods: <https://groups.google.com/d/msg/indigo-general/NBqNZ4Ik4-Y/goMuQxL9ZkEJ>

Indigo 1.1.3
----------

*23 August 2012*

* JNA has been updated to 3.4.1. This fixed an issue with permissions for the temporary directory.  
Thanks to Ingo: <http://tech.knime.org/forum/indigo/bundle-could-not-be-activated>
* Transformation method automatically calls a layout algorithm if necessary.
* Minor bug in the reaction exact matching algorithm has been fixed.
* Improvements in the layout of the atoms with four bonds attached.
* GrossFormula now uses Hill notation: <https://groups.google.com/d/msg/indigo-general/ntLPh-vz_P4/EQr-prG3gUEJ>
* Improvements in the SMARTS saving procedure.
* Molfile saver now correctly saves query bond topology constraints, unsaturation flag, and atom ring bond count constraint.
* Issues with rendering query bond topology constraints and atom ring bond count constraint have been fixed.
* Data SGroups with absolute coordinates are treated as relative for the layout procedure.
* SRU unit in the molfile now has a label.
* Issues causing infinite loop due to the numeric errors in the layout algorithm have been fixed.
* Issue with loading a molecule with 3D coordinates has been fixed.  
Thanks to Colin Batchelor: <https://groups.google.com/d/msg/indigo-bugs/rDsAJeDdNPo/Ca7RusLj8xYJ>
* Allene centers now are recognized if the angle between double bonds are greater than 165 degrees.

Indigo 1.1.2
----------

*10 July 2012*

* Layout algorithm now doesn't apply Fischer projection for atoms with 4 bonds. For example, now the CC(C)(C)C(C)(C)C(C)(C)C(C)(C)C molecule is cleaned up in a zigzag way.
* Bug with a missing stereocenter in the transformation and reaction product enumeration algorithms has been fixed:  <https://groups.google.com/d/msg/indigo-general/NkZ-g3EeuTg/FjqVjU4ZrYcJ>
* Layout algorithm for molecules with R-groups has been fixed.

Indigo 1.1.1
----------

*18 June 2012*

* symmetryClasses methods was added. Now the molecule object has a method symmetryClasses() that returns an array with a symmetry class ID for each atom.  
Thanks to Karen for the suggestion: <https://groups.google.com/d/msg/indigo-general/vR9BSWR87e8/PqpiQaE4SfgJ>
* Query molecules can now have a highlighting constraint on atoms and bonds to match only (un)highlighted target atoms or bonds. Here is an exmaple: query.getAtom(0).addConstraint("highlighting", "true").  
Again thanks to Karen: <https://groups.google.com/d/msg/indigo-general/J1RR9b0x2NM/Z_XOB9jQNw8J>

Indigo 1.1
----------

*07 June 2012*

* ChemDiff and Legio now supports the Indigo 1.1 version, installation scripts were fixed.


Indigo 1.1 Release Candidate 3
----------

*17 May 2012*

* Aromatic Te can be read from SMILES as [te]. Thanks to Andrew Dalke: <http://groups.google.com/d/msg/indigo-general/MlBa6Wc31L8/03i5Yfe0FP4J>
* Improvements in atom-to-atom mapping algorithm.

Indigo 1.1 Release Candidate 2
----------

*05 May 2012*

Fixed:

* Molecule with generic s-groups serialization
* Missed IndigoRenderer within Java bundle

Indigo 1.1 Release Candidate
----------

*30 April 2012*

Highlights:

* InChI stereochemistry layer is supported both for loading and saving molecules. The only difference 
with the standard utility occurs when stereochemistry is defined not in a proper way. Allenes and 
cumulenes are not supported yet.

* new RGroup-Decomposition API was added: createDecomposer(), addDecomposition(), decomposeMolecule(), iterateDecompositions()  
See more details at <http://groups.google.com/group/indigo-general/browse_thread/thread/75281df2f70ec1a>
Thanks to Gerhard: <http://groups.google.com/group/indigo-general/browse_thread/thread/c1dbc67ece5f78b0>,  
Mederich: <http://groups.google.com/group/indigo-general/browse_thread/thread/6d77029359364dd8>, and  
Simon: <http://tech.knime.org/forum/indigo/r-group-decomposer>

* We completely switched to CMake project configurations.

Changes:

* AAM new algorithm heuristic was implemented for disconnected reactant and product molecules.
* correctReactingCenters() method was added for reactions. It highlights bond reacting centers according to AAM.  
Thanks to James: <http://tech.knime.org/forum/indigo/reaction-automapper-bond-highlighting>
* "timeout" option is used for MCS computation.

Fixes:

* The bug with aam for query reactions was fixed
* The bug with aam timeout was fixed.  
Thanks to Daniel: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/1cc5b9dffd740240>
* clearStereocenters() method now resets bond directions. After calling this method molecule is saved into Molfile format without tetrahedral bond directions.
* Exception during saving Molfile with pseudoatoms within aromatic rings
* Exception when loading a molecule from Molfile with 3D coordinates with invalid valences during automatic stereocenters detection.
* Some other issues.

Indigo 1.1-beta10
----------

*29 March 2012*

Changes:

* IndigoObject is Java now have dispose() method to dispose Indigo object before garbage collection.
* Molfile atom lists now support pseudoatoms
* Global timeout for all the most time consuming operations: substructure search, canonical smiles generation and etc. Option is called "timeout" and corresponts to milliseconds.
* explicit hydrogen near Nitrogen is handled correctly to calculate cis-trans and tetrahedral stereo configuration.
* InChI plugin now have "version" methods to return an actual InChI implementation version
* Arial font is used on Linux systems to render molecules. Previously this font was used only on Windows and Mac OS X, and rendered images on Windows and Linux were different.
* "deco-ignore-errors" option was added. Now there are no exceptions like 'no embeddings obtained' during the RGroup Decomposition if the flag set true. Exception is raised only for the end getters (e.g. decomposedMoleculeWithRGroups())
* "deco-save-ap-bond-orders" option was added. Within the option output molecule RGroup attachment points are saved as pseudo atoms (named 'AP1', 'AP2' etc). Therefore, the option allows to save initial bond orders.  
Thanks to Mederich: <http://groups.google.com/group/indigo-general/browse_thread/thread/c4bca8b97ca54a87>
* bug with the time hang was fixed for AAM.  
Thanks to Daniel: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/1cc5b9dffd740240>
* minor bug fixes in AAM
* minor bug fixes in RGroup Decomposition

Fixed:

* automatic 2D coordinates generation procedure (layout) changes molecule components position if they have fixed atoms
* cycle enumeration fixed.  
Thanks to Casey: <https://groups.google.com/d/msg/indigo-general/UPkiBz1e-_o/WMtKB9RGE-UJ>
* memory leak in the InChI computation procedure.  
Thanks to Hinnerk: <https://groups.google.com/d/msg/indigo-bugs/Fvr4l8CQvAQ/r_HYDxumALAJ>
* different minor exception when loading a molecule from a molfile
* different minor exception when rendering a molecule


Indigo 1.1-beta9
----------

*25 February 2012*

Changes:

* if a molecule contains only R-group #2 then empty R-rgroup #1 is not rendered any more.
* molecules with bad valences and charges can be serialized now
* timeout option was added for AAM. A new option was added named "aam-timeout". The integer parameter (time in milliseconds) corresponds for the AAM algorithm working time. The automap method returns a current state solution for a reaction when time is over. Thanks to Daniel: <http://groups.google.com/group/indigo-dev/browse_thread/thread/4430412b9864f3fd>
* default layout call was added for the deconvolultion scaffold getter (decomposedMoleculeScaffold())
* empty RGroup handling (one single bond) was implemented for deco.
* minor bug fixes in AAM
* minor bug fixes in RGroup Decomposition


Fixed:

* incorrect empty R-Group logic loading from molfile
* incorrect attachmement points loading from molfile if the number of attachments points is greater then 2
* memory leak in reaction substructure matcher.
* infinite loop in reaction substructure matcher.  
Thanks to fab for the bug report for both issues: <http://tech.knime.org/forum/indigo/error-in-loop>
* invalid stereo configuration when atom are being changed.  
Thank to Lionel for the bug report: <http://tech.knime.org/forum/indigo/changes-in-molecule-properties-node>
* bug with AAM not respecting atom type.  
Thanks to Daniel: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/9448f08ab596b74e>


Indigo 1.1-beta8
----------

*29 January 2012*

We have released our first version of InChI plugin that allows to load InChI strings and generate InChI and InChIKey for molecules (this version discards stereoinformation, but we are working on it). The plugin is statically linked with the official InChI library and can be loaded on demand, as it is done with IndigoRenderer plugin.

Usage example :

    IndigoInchi inchi = new IndigoInchi(indigo);
    IndigoObject molecule = indigo.loadMolecule("InChI=1S/C3H9NO/c1-3(5)2-4/h3,5H,2,4H2,1H3");
    String inchi_string = indigo.getInchi(molecule);

New methods and functionallity:

* InChI support! (without stereochemistry yet)
* mapMolecule(queryReactoinMolecule) to retrieve mapped molecule for the query reaction for the reaction substrcuture match object
* getMolecule(index) to get the reaction molecule
* QueryMolecules can now be constructed with the following methods:

    1. addAtom, resetAtom methods for the QueryMolecule now parses arbitrary SMARTS
    2. addBond method for QueryMolecule
    3. atom.addConstraintOr method has been added
    4. a lot of query atom constraints: atomic-number, charge, isotope, radical, valence, connectivity, total-bond-order, hygrogens, substituents, ring, smallest-ring, ring-bonds, rsite-mask

Fixed: 

* Issue with loading molecule attachment points if the bond orders are not marked.
* Better handling of molecules with invalid valence: canonical SMILES, unfoldHydrogens, invalid stereocenters detection. Thanks to Mederich for the bug report: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/8f1ac4c1bfcbc346>
* Molecule serialization with more than 8 R-groups. Thanks to James Davidson for the bug report: <http://tech.knime.org/forum/indigo/changes-to-scaffold-finder-node>


Indigo 1.1-beta7
----------

*29 December 2011*

Changelog:

 * Fixed bug: render-grid-title-offset options is not initialized.
 * Fixed bug: all images are rendered as grid, after grid has been rendered.
 * Possible memory issue in IndigoRenderer for Java has been fixed.
 

Indigo 1.1-beta6
----------

*12 December 2011*

New functionality:

 * Indigo.transform(reaction, molecule) method for transformation a molecule according  to a rule, specified with a reaction.  
   Examples are available here: <http://ggasoftware.com/opensource/indigo/concepts/transformation>

 * New IndigoObject methods for working with reaction atom-to-atom mapping: atomMappingNumber, setAtomMappingNumber, clearAAM
 
 * New IndigoObject methods for working with attachment points: iterateAttachmentPoints, countAttachmentPoints, clearAttachmentPoints.  
   See <http://ggasoftware.com/opensource/indigo/api#attachment-points> for more details
 
 * Other new IndigoObject methods with documentation has been added: changeStereocenterType, addStereocenter, reactingCenter, setReactingCenter, loadSmartsFromFile, loadReactionSmartsFromFile, getSuperatom, getDataSGroup, description, decomposedMoleculeHighlighted, getSubmolecule, addSuperatom
 
 * Smiles saver might throw an exception on a molecule with explicit hydrogens.  
   Thanks to Colin Batchelor: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/35b240fb402e35c3>
  
Changelog:

 * Improvements in the automatic atom-to-atom assignment.  
   Thanks to Ernst-Georg Schmid: <http://groups.google.com/group/indigo-general/browse_thread/thread/ffe48381a01f7d24>
   And to Daniel Lowe:
<http://groups.google.com/group/indigo-bugs/browse_thread/thread/11373837ba65acd>
 
 * Improvements in the molecule decomposition algorithm.
 
 * Python 2.4 support.
 
 * A lot of bugs has been fixed due to some internal inconsistency in explicit hydrogens handing for cis-trans bonds:
 
      - Substructure matcher result can be incorrect for matching query molecule with cis-trans bonds.
      - Substructure matcher result can be incorrect in case of explicit hydrogens for cis-trans bonds in the target molecule.
      - If a molecule has explicit hydrogens near cis-trans bonds, after been unserialized cis-trans configuration might flip.
      - Canonical SMILES may also produce different results for a molecule with explicit hydrogens and without them.  

		
 * Better stability of Indigo Java wrapper	
 
 * Better rendering of a SMARTS query molecules
 
 * Indigo now informs with an exception that both cis- and trans- specification in the SMARTS expression is not supported yet. For example, such SMARTS is not supported yet: `*/,\[R]=;@[R]/,\*`
 
 * Fixed issue with saving query molecules in Molfile format with the atom lists.  
   Thanks to Francesca: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/b17b468049caf57a>
 
 * unfoldHydrogens how works with reaction properly.
 
 * Some fixes of the dearomation algorithm bug arisen during tautomer substructure matching.
 
 * Better support of sgroups in Molfile
 
 * Highlighting is taken into account for the computation of canonical SMILES
 
 * Indigo.countHydrogens method doesn't throw an exception is case of existence of R-groups and pseudoatoms.
 
 * Fixed some issues with loading and saving of polymer repetition in SMILES
 
 * SGroups and R-sites are saving during serialization/unserilization.  
   Thanks to Hinnerk Rey: <http://groups.google.com/group/indigo-general/browse_thread/thread/1d9bda07b8ac299d>
 
 * Faster matching of SMARTS queries with unspecified bonds. This change also improves efficiency for our fingerprints for query molecules.
 
 * Substructure matching counter now property counts queries with explicit hydrogens, like N-[#1].  
   Thanks to James Davidson for this bug report: <http://tech.knime.org/forum/indigo/substructure-match-counter-question>
 
 * Stereocenter parities are now saved into molfile.  
   Thanks to Lionel: <http://tech.knime.org/forum/indigo/chirality-flags>
 
 * R-group iterator now skips R-groups that are empty.
 
 * Molfile loader now accepts left- and right-bounded atom labels.  
   Thanks to Ernst-Georg Schmid: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/1d2b8a01af98949>
 * renderGridToBuffer method now support null value as the second parameter.  
   Thanks to Mederich: <http://groups.google.com/group/indigo-general/browse_thread/thread/b995c53227cf3352>


Indigo 1.1-beta5
----------

*11 August 2011*

New functionality:

  * Methods for specifing reacting centers on bonds: reaction.reactingCenter(bond), reaction.setReactingCenter(bond, mask)  
    All reacting centers types are describes in Indigo namespace for Java and Python, and in ReactingCenter enum for C#.  
    Code examples can be found in this thread: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/11373837ba65acd>
    
  * Method to add stereocenter at atom: atom.addStereocenter(type, atom_index1, atom_index2, atom_index3, atom_index4). Last parameter is optional.  
    Code examples can be found in this thread: <http://groups.google.com/group/indigo-dev/browse_thread/thread/a164eddce485f053>

Note: this new methods have preliminary interface, and interface may be changed in the next version.

Fixed:

  * Molecule to Smiles conversion with explicit hydrogens connected to cis-trans bonds.  
    Thanks to Colin Batchelor: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/35b240fb402e35c3>

Indigo 1.1-beta4
----------

*29 July 2011*

New functionality:

  * New methods for Indigo: resetAtom, setRSite, isHighlighted for atoms.  
    Code example: atom.resetAtom("N"), atom.setRSite("R1, R3"), atom.isHighlighted()
  * Reaction product enumerator now supports recursive SMARTS

Fixed:

  * Exception during Indigo for Java and Indigo for Python initialization on Mac OS X Lion 10.7
  * Different AAM issues. Thanks to Daniel Lowe: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/11373837ba65acd>
  * Exception when calling hasCoord and hasZCoord on a reaction object
  * Reaction product enumerator exception when monomers have no name
  
Indigo 1.1-alpha3
----------

*7 July 2011*

New functionality since Indigo-1.0.0 stable version:

  * atomMappingNumber and setAtomMappingNumber methods for atoms to
    retrieve and change atom-to-atom numbers. New clearAAM method to
    clear atom-to-atom mapping information. Thanks to Daniel Lowe
    for pointing out that this functionality is missing. Code
    examples can be found in this thread: <http://groups.google.com/group/indigo-general/browse_thread/thread/d8a413a88b9da834>
  * addRSite method for adding R-site atoms to the molecule. This
    method is similar to addAtom.  
    Code example: atom = mol.addRSite("R1")

Fixed:

  * foldHydrogens on [H][H] and molecules with isotopic
    hydrogens ([2H]C). Thanks to Daniel Lowe: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/2a8416c875aa8fb>
  * Reaction layout for reactions with empty reactants
  * Saving molecule with s-group into molfile format
  * Substructure matcher with special query with recursive smarts
    beginning with hydrogen
  * Unbounded memory usage during reaction automapping.
    Thanks to Daniel Lowe again: <http://groups.google.com/group/indigo-bugs/browse_thread/thread/e6a5e0430032e1a6/9dc36a81491283d0>
  * Indigo Python API module loading on Mac OS X from different
    directories might cause error messages
  * Reaction substructure match throws an exception in some cases
    when these is no pair of AAM numbers. For example, reactant
    has AAM number while product hasn't it.
