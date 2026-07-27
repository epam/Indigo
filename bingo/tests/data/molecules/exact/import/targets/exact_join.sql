/*
Thanks to amhuhn2 for providing this test case
This test case is almost blindly copied from 
https://github.com/epam/Indigo/issues/2922
to cover this particular issue
*/

/* Create a new schema */
DROP SCHEMA IF EXISTS issue2922 CASCADE;

CREATE SCHEMA IF NOT EXISTS issue2922;

/* Create table compound */
DROP TABLE IF EXISTS issue2922.compound;

CREATE TABLE issue2922.compound
(
	compound_id INTEGER NOT NULL,
    mol_file text,
    PRIMARY KEY (compound_id)
);

ALTER TABLE IF EXISTS issue2922.compound
    OWNER to postgres;

/* Create a bingo index on table compound */
CREATE INDEX IF NOT EXISTS idx_compound
    ON issue2922.compound USING bingo_idx
    (mol_file bingo.molecule);

/* Create table compound_security */
DROP TABLE IF EXISTS issue2922.compound_security;

CREATE TABLE issue2922.compound_security
(
    compound_id INTEGER NOT NULL,
    PRIMARY KEY (compound_id)
);

ALTER TABLE IF EXISTS issue2922.compound_security
    OWNER to postgres;

/* Populate both tables with one row each */
insert
  into issue2922.compound_security
     ( compound_id )
values
     ( 736832 );

insert
  into issue2922.compound
     ( compound_id, mol_file )
values ( 736832, 'Unnamed
MolEngine06122512002D

  0  0        0               999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 14 13 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 14.907 -3.539 0 0
M  V30 2 C 16.258 -2.759 0 0
M  V30 3 H 16.258 -1.199 0 0
M  V30 4 C 17.609 -3.539 0 0
M  V30 5 C 18.96 -2.759 0 0
M  V30 6 H 18.96 -1.199 0 0
M  V30 7 H 20.311 -3.539 0 0
M  V30 8 H 17.609 -5.099 0 0
M  V30 9 H 14.907 -5.099 0 0
M  V30 10 H 13.556 -2.759 0 0
M  V30 11 H 14.907 -1.979 0 0
M  V30 12 H 16.258 -4.319 0 0
M  V30 13 H 17.609 -1.979 0 0
M  V30 14 H 20.311 -1.979 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 2 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 5 7
M  V30 7 1 4 8
M  V30 8 1 1 9
M  V30 9 1 1 10
M  V30 10 1 1 11
M  V30 11 1 2 12
M  V30 12 1 4 13
M  V30 13 1 5 14
M  V30 END BOND
M  V30 END CTAB
M  END' );

/* The following fails with this error:
ERROR:  could not find block containing chunk 0x64fdbb8 
SQL state: XX000

It is attempting to select from a JOIN of the two tables,
using the bingo index in the WHERE clause.
*/

SELECT c.compound_id as c
  FROM issue2922.compound AS c
INNER JOIN
       issue2922.compound_security AS cs
    ON c.compound_id = cs.compound_id
 WHERE (c.mol_file @ ('Unnamed
MolEngine06122512002D

  0  0        0               999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 14 13 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 14.907 -3.539 0 0
M  V30 2 C 16.258 -2.759 0 0
M  V30 3 H 16.258 -1.199 0 0
M  V30 4 C 17.609 -3.539 0 0
M  V30 5 C 18.96 -2.759 0 0
M  V30 6 H 18.96 -1.199 0 0
M  V30 7 H 20.311 -3.539 0 0
M  V30 8 H 17.609 -5.099 0 0
M  V30 9 H 14.907 -5.099 0 0
M  V30 10 H 13.556 -2.759 0 0
M  V30 11 H 14.907 -1.979 0 0
M  V30 12 H 16.258 -4.319 0 0
M  V30 13 H 17.609 -1.979 0 0
M  V30 14 H 20.311 -1.979 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 2 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 5 7
M  V30 7 1 4 8
M  V30 8 1 1 9
M  V30 9 1 1 10
M  V30 10 1 1 11
M  V30 11 1 2 12
M  V30 12 1 4 13
M  V30 13 1 5 14
M  V30 END BOND
M  V30 END CTAB
M  END', '0.1')::bingo.exact) = TRUE;
