Bingo 1.7.9
----------

*3 April 2013*

General changes:

* Fingerprints now include information about single-atom fragments. This resolved an issue with empty fingerprints for single-atom molecules. Also this change makes substruture search more efficient in case of rare atoms in a query.
* Timeout parameter for exact search method for a single structure. Default value is 1 min.
* Smiles saver doesn't throw exception about implicit hydrogens if they are not saving in SMILES

Bingo PostgreSQL-specific changes:
* bug with similarity search for null structures was fixed
* performance was improved for join queries
* bug with exact join query (windows) was fixed 
* bitmap scan feature was turned off by default for bingo

Bingo 1.7.8b2
----------

*15 February 2013*

General changes for Bingo Oracle, SQL Server and PostgreSQL:

* all the bug fixes from the Indigo branch (AAM cancellation timeout, aromatization bug fixed and others)

Bingo PostgreSQL-specific changes: 

* import sdf bug was fixed (throw error on incorrect format)
* bug with the rescan queries was fixed (join queries now work properly)
* errors while binary matching were fixed 
* multi-threading was implemented for index building (creating index performance was improved)


Bingo 1.7.7
----------

*29 January 2013*

This is a stable Bingo release for the all three databases: Oracle, MS SQL Server, and PostgreSQL. It includes all the bug fixes from the Indigo branch.

New methods for Bingo Oracle, SQL Server and PostgreSQL:

* `bingo.InChI(molecule, options)` method to compute InChI identifier. You can pass any options that are supported by InChI library via `options` argument. For example you can provide options in the following way:

        bingo.InChI(molecule, '/DoNotAddH /SUU /SLUUD')

* `bingo.InChIKey(inchi)` method to compute InChI Key. To get an InChI key from a molecule with a default options use the following syntax:

        select bingo.InChIKey(bingo.InChI(molecule, ''))

* `bingo.fingerprint(molecule, options)` and `bingo.rfingerprint(reaction, options)` methods to compute fingerprints. The options are the same as we have for the IndigoObject.fingerprint method in Indigo:
  * sim - "Similarity fingerprint", useful for calculating similarity measures (the default)
  * sub - "Substructure fingerprint", useful for substructure screening 
  * sub-res - "Resonance substructure fingerprint", useful for resonance substructure screening
  * sub-tau - "Tautomer substructure fingerprint", useful for tautomer substructure screening
  * full - "Full fingerprint", which has all the mentioned fingerprint types included

Bingo PostgreSQL-specific changes: 

* Postgres 9.2 support was added
* Bug with cost estimation engine was fixed
* `bingo.compactMolecule(molecule, xyz)` and `bingo.compactReaction(reaction, xyz)` methods were added
* All utility methods now return text (not `cstring`)
* All utility methods now accept binary byte parameter for input structures(returned by `bingo.compactMolecule()` and `bingo.compactReaction()`)

Bingo Oracle-specific changes: 

* rowid is includeed into the message in case of exceptions

Bingo SQL Server-specific changes: 

* primary keys are added to the Bingo-specific table to support replication procedure.
* more details in the diagnostics log: thread id, process id, and etc.
* better support for query cancellation

Also we switch to a continues version numbering like we are doing with Indigo releases.


Bingo 1.7 beta4
----------

*24 April 2012*

All the improvements have been merged from the Indigo branch (versions above 1.1 beta6), including: 

* Better handling of molecules with invalid valence. 
* Molecule serialization with more than 8 R-groups 
* incorrect empty R-Group logic loading attachment points loading from molfile was fixed 

The new release contains bingo-specific changes including critical bugfixes for PostgreSQL:

* Bug with inserting to a table contained more than 64k molecules was fixed. 
* Build and search queries now support cancel requests.
* Bug with possible dead lock for PostgreSQL hang queries was fixed. 
* Binary data storing was fixed.
* Possible segmentation fault during raising exceptions was fixed. 
* The issue with queries contained several bingo functions was resolved. 

Also, bug with Oracle 11.2 on Windows 7 x64 was fixed. 

Bingo 1.7 beta3
----------

*28 December 2011*

All the bugfixes and improvements from the Indigo branch have been merged into the Bingo branch including: 

* Cis-trans issues has been fixed. 
* More accurate query fingerprints for SMARTS queries. 

Bingo-specific changes: 

* Support of the PostgresSQL 9.1 
* Now binary files for Oracle are available for download, because we eliminated a linkage with OCI libraries. No building from source is required any more for getting Bingo for Oracle on Windows. 

Bingo 1.7 beta2
----------

*30 November 2011 (no public announcement)*

Bugfix release.

Bingo 1.7 beta1
----------

*29 September 2011*

First Bingo version for PostgreSQL. The cartridge is covered 
almost all the functionality taken from the Bingo Oracle and the Bingo 
Sql Server parts. 

The Bingo library for the PostgreSQL supports index building for 
molecules and reactions. The algorithm for the index search includes 
both the basic Bingo search engine principles and the unique heuristic 
algorithms for the PostgreSQL database. Thus the PostgreSQL Bingo 
cartridge can show a great performance comparable to or greater than 
other Bingo implementations. 

Bingo 1.6.0
----------

*17 June 2011*

The stable Bingo release.
