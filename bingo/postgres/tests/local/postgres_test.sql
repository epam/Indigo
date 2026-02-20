select 'CC' @ ('CC', '')::bingo.sub

select 'CC' @ ('CC', '')::bingo.smarts

explain select * from btest where a @ ('CC(=O)', '')::bingo.sub

select * from bingo.bingo_config
drop index btest_idx;
create index btest_idx on btest using bingo_idx (a bingo.molecule) with ( REJECT_INVALID_STRUCTURES=1)
create index btest_idx on btest using bingo_idx (a bingo.molecule) with (NTHREADS=1)
create index rtest_idx on rtest using bingo_idx (a bingo.reaction)

select bingo.CheckMolecule('[O-][N+](=O)c1cc([N+](=O)[O-])c2c(c1)[n+]1nc3c([n-]1n2)cc(cc3[N+](=O)[O-])[N+](=O)[O-]');

create index btest_idx on btest using bingo_idx (a bingo.molecule) with (IGNORE_CISTRANS_ERRORS=1)

DROP TABLE bingotest.bingo_test_table3

SELECT * FROM bingotest.bingo_test_table2 where a @ ('OCNCCl', 'ELE')::bingo.exact

SELECT bingo.getIndexStructuresCount('bingotest.bingo_test_index2'::regclass::oid)

update bingo.bingo_config set cvalue='1' where cname='STEREOCHEMISTRY_DETECT_HAWORTH_PROJECTION'

select 'NC1=CC=CC(=C1)C1=CC=CC=C1' @ ('=', 'C12H11N')::bingo.gross

select 'NC1CCCC(C1)C1CCCCC1' @ ('CC1CCCCC1', '')::bingo.smarts


explain select * from btest where a @ cast(('CC(=O)', '') as bingo.sub)

select * from btest where a @ ('NC(=O)', '')::bingo.sub
select * from btest where a @ ('OC1=CC=CC=C1', '')::bingo.sub

drop table aatest
truncate table btest
select bingo.importSDF('btest(a)', '/home/tarquin/projects/indigo/indigo-git/bingo/tests/postgres/java_tests/test_mango.sdf')
create table aatest(a text)
insert into aatest (select * from btest where a @ ('CC1CCCCC1', '')::bingo.smarts)
explain select * from aatest where not bingo.matchSmarts(a, ('CC1CCCCC1', ''))

create index aatest_idx on aatest using bingo_idx (a bingo.molecule)


select count(*) from aatest where bingo.matchSmarts(a, ('CC1CCCCC1', ''))
select a @ ('CC1CCCCC1', '')::bingo.smarts, bingo.matchSmarts(a, ('CC1CCCCC1', '')) from btest


select * from btest where  @ ('CC(=O)', '')::bingo_exact
select * from btest where a @ ('CCC', '')::bingo_exact
explain select * from btest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo.exact
select * from btest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', 'TAU')::bingo.exact

select * from rtest where a @ ('OC1=CC=CC=C1>>', '')::bingo.rsub
explain select * from rtest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>', '')::bingo.rexact

explain select a, bingo.Sim(a, 'Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '') from btest 
where a @ (0.9, 1, 'Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo.sim

select a from btest where a @ (0.9, 1, 'Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo.sim

select bingoSim('CCCCS(=N)(=O)CC[C@@H](N)C(O)=O', a, '') from test_table

select * from btest where a @ ('>=', 'C3')::bingo_gross

insert into btest(a) values('CCC');

vacuum btest

select 'CC' @ 'CC'
select ('CC', '')::molquery @ 'CC'
drop table btest_idx_shadow

select * from btest
select * from rtest_idx_shadow
delete from btest where a='CCC'
truncate table btest
select * from btest where ctid='(0,97)'::tid

drop table btest_idx_shadow
select * from btest_idx_shadow
select * from btest_idx_shadow_hash
create index btest_idx on btest using bingo_idx (a bingo_ops) with ( "treatx-as-pseudoatom=5", FP_SIM_SIZE=4);
create index btest_idx on btest using bingo_idx (a bingo.molecule) with (NTHREADS=1)

explain select * from btest where a @ ('CCC', '')::bingo.sub
explain select * from btest where a @ ('CC', '')::bingo_exact
explain select * from btest where ('CC', '')::molquery @ a
explain select * from btest where a @ 'CC'
279431
CREATE RULE prot_btest_rule AS ON select to btest where a @ ('', '')::bingo_sub 
do instead select * from btest where a @ ('CC', '')::bingo_sub

set INDEX_INHERITANCE_SUPPORTED = true
select * from inheritance



select * from pg_depend
select count(*) from pg_depend
select * from pg_class where relname = 'btest'
select relname from pg_class where relfilenode = 402148
select relname from pg_class where oid = 2200

select 'btest'::regclass::oid
select 'btest_idx'::regclass::oid

select * from pg_depend where objid='btest'::regclass::oid
select * from pg_depend where objid='btest_idx'::regclass::oid
select * from pg_depend where objid='btest_idx_shadow'::regclass::oid

insert into pg_depend (classid, objid, objsubid, refclassid, refobjid, refobjsubid, deptype)
values (
'pg_class'::regclass::oid,
'btest_idx_shadow'::regclass::oid,
0,
'pg_class'::regclass::oid,
'btest_idx'::regclass::oid,
0,
'i')


drop index btest_idx2;
create index btest_idx2 on btest using bingo_idx (a bingo_sim_ops);


select * from pg_proc where proname in ('hashcostestimate','bingo_costestimate')
select * from pg_proc where proname in ('hashbuild','bingo_build')
select * from pg_language where oid = 12


select * from btest where a @ 'NC'


insert into test_table(a) values('CCC')
select * from test_table
select pg_size_pretty(pg_total_relation_size('btest'))
vacuum test_table

truncate table test_table

drop index bingo_test_index
create index bingo_test_index on test_table using bingo_idx (a bingo_ops)

explain select * from btest where a @ 'C1CCCCC1'
explain select * from test_table where a @ 'C1CCCCC1'

select bingo_sim('CCC=O', 'CC');

select proname, prorettype, proargtypes, prolang from pg_proc where proname like 'hash%'
select * from pg_type where oid = 279431
select * from pg_am

explain select mol from test_table where (mol @ 'CCC') is true;

create index test_idx on test_table using gist (mol gist_bingo_ops);

CREATE OR REPLACE FUNCTION bingo_test(text)
RETURNS float4
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres'
LANGUAGE C STRICT IMMUTABLE;

select bingo_test('5')


drop function bingo_costestimate(internal, internal, internal, internal, internal, internal, internal, internal)
select 'CC' @ ('CCX', '')::bingo.sub
select 'bingo_insert(internal, internal, internal, internal, internal)'::regprocedure::oid

select oid from pg_proc where proname='bingo_insert'

select * from pg_am where amname in ('hash', 'bingo_idx')



drop table btest;
create table btest(a text, b serial);

insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');

insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('AAA');
drop table btest
drop table rtest
create table rtest(a text);

insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12>>');
insert into rtest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3>>');
insert into rtest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C>>');
insert into rtest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4>>');
insert into rtest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3>>');

select * from pg_class where relname='btest_idx'
select oid from pg_class where relname='btest_idx'
select 'btest_idx'::regclass::oid
select * from pg_opclass 
32776
update pg_class set relam=32776 where relname='btest_idx'



select bingoImportSDF('btest'::regclass::oid, '2', '3')
REINDEX DATABASE test

select * from test_table
select * from test_table where a @ ('C1CCCCC1', '')::bingo_sub

select count(*) from btest


select * from btest

truncate table btest

select bingoImportSDF_sql('btest(a)', '/home/tarquin/projects/indigo/postgres-git/tests/test_import.sdf')

drop table test_table
create table test_table(a text)
select bingoImportsdf1('test_table(a)', '/home/tarquin/projects/indigo/postgres-git/tests/test_import.sdf')



drop FUNCTION bingo_getStructuresCount(oid)
select bingoGetIndexStructuresCount('btest_idx'::regclass::oid)

create table acd2d_symyx(a text)
select count(*) from acd2d_symyx
insert into test_table(a) values(null)
truncate table btest
select bingo.importSDF('acd2d_symyx(a)', '/home/tarquin/projects/bases/acd2d_symyx.sdf')

grant all on table pg_depend to tarquin
create index acd2d_idx on acd2d_symyx using bingo_idx (a bingo.molecule)
select * from acd2d_symyx where a @ ('COCC1=CC=C(C=C1)C1=CC=CC=C1','')::bingo.smarts limit 100;
select * from pg_depend

select count(a) from btest
        
select * from btest limit 100
drop table btest_shadow
create table btest_shadow(idx tid, h integer)
select * from btest_shadow 
select count(*) from btest_shadow
select a from btest
select '(1,2)'::tid
insert into btest_shadow values()


create index btest_shadow_index on btest_shadow(idx)
explain select * from btest_shadow where idx = '(1,2)'::tid

CREATE OR REPLACE FUNCTION bingo_test_tid()
RETURNS tid
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_test_select()
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres'
LANGUAGE C STRICT IMMUTABLE;



CREATE OR REPLACE FUNCTION bingo_test_tid_insert()
RETURNS boolean AS $$
   DECLARE
      sc tid;
   BEGIN
	execute 'insert into btest_shadow values('|| CAST ( bingo_test_tid() AS tid ) ||', 2)';
	RETURN true;
   END;
$$ LANGUAGE 'plpgsql';

select bingo_test_tid()
select bingo_test_tid_insert()
select * from btest_shadow
select ctid, a from btest


CREATE OR REPLACE FUNCTION bingo_test_cur_begin(text)
RETURNS refcursor AS $$
  DECLARE
    ref refcursor;
  BEGIN
    OPEN ref FOR EXECUTE 'SELECT idx FROM ' || $1;
    RETURN ref; 
  END;
$$ LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION bingo_test_cur_next(refcursor)
RETURNS tid AS $$
  DECLARE
    res tid;
  BEGIN
    FETCH $1 into res;
    if res is null then
      res := '(0,0)'::tid;
    end if;
    RETURN res; 
  END;
$$ LANGUAGE 'plpgsql';


CREATE OR REPLACE FUNCTION bingo_execute_func(text)
RETURNS void AS $$
  BEGIN
    EXECUTE $1;
  END;
$$ LANGUAGE 'plpgsql';

select bingo_test_cur_next(bingo_test_cur_begin('btest'))
select bingo_test_select()

truncate table btest_shadow
insert into btest_shadow values('(1,4)'::tid, 2)

select * from btest_idx_shadow
create table btest_idx_shadow(like btest_idx) 
drop table btest_idx_shadow
create table btest_idx_shadow (b real) inherits (btest_idx)

create table btest_idx(a integer)
select * from btest_idx
drop table btest_idx cascade
insert into btest_idx(a) values(5)
truncate table btest_idx
delete from btest_idx where a = 5


CREATE OR REPLACE FUNCTION check_idx_trigger() RETURNS "trigger" AS $$
begin
 drop table if exists btest_idx_shadow;
 return old;
end;
$$ LANGUAGE 'plpgsql' VOLATILE;

drop trigger if exists  check_idx_trigger on btest_idx

CREATE TRIGGER check_idx_trigger BEFORE TRUNCATE OR DELETE OR UPDATE
  ON btest_idx 
  EXECUTE PROCEDURE check_idx_trigger();

CREATE CONSTRAINT TRIGGER check_idx_trigger2
    AFTER TRUNCATE ON
    btest_idx EXECUTE PROCEDURE check_idx_trigger()


select * from test_table where a @ ('C1CCCCC1', '')::bingo_sub
drop index  bingo_test_index
create index bingo_test_index on test_table using bingo_idx (a bingo_ops)

select bingoMass('CCC')
select bingoMassType('CCC', 'monoisotopic-mass')

SELECT relname, relkind, reltuples, relpages
FROM pg_class
WHERE relname LIKE 'btest%';


truncate table test_table
select bingoImportSDF('test_table(a)', '/home/tarquin/projects/indigo/postgres-git/tests/test_import.sdf')
delete from test_table where a = 'OCNCCl'
vacuum analyze test_table
select * from test_table where a @ ('CCCCS(=N)(=O)CCC(N)C(O)=O', 'ELE')::bingo_exact
select bingoGetIndexStructuresCount('bingo_test_index'::regclass::oid)





select 'CCC' < '50'::bingo_mass
explain select * from btest where a > '100'::bingo_mass and a < '300'::bingo_mass OR a @ ('CCC', '')::bingo_sub


CREATE OR REPLACE FUNCTION bingo_test()
RETURNS void
AS '$libdir/bingo_postgres'
LANGUAGE C STRICT IMMUTABLE;

create schema bingo
drop schema bingo cascade
delete from pg_am where amname='bingo_idx'
create table bingo.test(a integer)

create table ringo_test(a text)
create index ringo_test_idx on ringo_test using bingo_idx (a bingo.reaction)
insert into ringo_test(a) values('OCNCCl>>OCNCCl')
insert into ringo_test(a) values('AAA>>')

CREATE OR REPLACE FUNCTION insert_table(text, integer) RETURNS void AS $$
declare idx integer;
begin
 idx := 0;
 LOOP
    INSERT INTO chembl_test2(data) values ($1);
    idx := idx + 1;
    EXIT WHEN idx > $2;
    
 END LOOP;
end;
$$ LANGUAGE 'plpgsql' ;

drop function insert_table(text, integer)

select insert_table('CCC', 129000)

create table test64k (a text)
drop table test64k
select count(*) from test64k
create index test64k_idx on test64k using bingo_idx (a bingo.molecule)
drop index test64k_idx
drop function bingo.getversion(oid)
select * from test64k where a @ ('CC', '')::bingo.sub limit 100

select bingo.getversion()

create table ctest(a text, b text)
truncate table ctest
select * from ctest
select bingo.importSDF('ctest', 'a', '', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/test_import.sdf')
select count(*) from ctest


create table test_sdf(a text);
select bingo.importSDF('test_sdf', 'a', '', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/data/test_import.sdf');
drop table test_sdf;
create table test_sdf(a text, b text);
select bingo.importSDF('test_sdf', 'a', 'MOL_ID b', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/data/test_import.sdf');
select * from test_sdf

create table test_rdf(a text);
select bingo.importRDF('test_rdf', 'a', '', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/data/test_import.rdf');
drop table test_rdf;
select * from test_rdf
create table test_rdf(a text, b text);
create table test_rdf(a text, b int);
select bingo.importRDF('test_rdf', 'a', 'MOL_ID b', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/data/test_import2.rdf');
create table test_smiles(a text, a_id text);
select bingo.importSmiles('test_smiles', 'a', 'a_id', '/home/tarquin/projects/indigo/indigo-git/tests/postgres.as/data/test_import.smi');
select * from test_smiles
truncate table test_smiles

select * from bingo.bingo_config
select * from bingo.bingo_config where cname='TREAT_X_AS_PSEUDOATOM'
update bingo.bingo_config set cvalue='1' where cname='TREAT_X_AS_PSEUDOATOM'
select bingo.getSimilarity('CCX', 'CC', '')
select bingo.smiles('AAA')

select bingo.getMass('AAA')

select bingo.gross('CCC')

create table test_bug1(a text)
select bingo.importSDF('test_bug1222', 'a', '', '/home/tarquin/projects/indigo/indigo-git/bingo/tests/data/molecules/exact/import/targets/mols.sdf');
select count(*) from test_bug1
create index test_bug1_idx on test_bug1 using bingo_idx (a bingo.molecule)
select a from test_bug1 where a @ ('CC(C)(C)[Si](C)(C)OCCN1CCOC1=O', '')::bingo.exact;
select a from test_bug1 where a @ ('CCC', '')::bingo.sub;


grant all on schema bingo to usert
grant select on table bingo.bingo_config to usert
grant select on table bingo.bingo_tau_config to usert

drop user usert 
revoke all on database test from usert
revoke all on schema public from usert
revoke all on table pg_depend from usert
revoke all on table bingo.bingo_config from usert
revoke all on schema pg_catalog from usert




select * from pg_depend
create table test1(a text);
create table test2(a text);
drop table test1
drop table test2
select bingo._internal_func_011(0, 'test1', 'test2')

select bingo.fileToText('/tmp/1.txt')
select bingo.fileToBlob('/tmp/1.txt')

drop table molecule_test;
create table molecule_test (a_id serial, a text); 

truncate table molecule_test

select bingo.importSmiles('molecule_test', 'a', '', 
'/home/tarquin/projects/indigo/indigo-git/bingo/tests/data/molecules/bigtable/import/targets/pubchem_slice_100000.smiles');
select * from molecule_test limit 100
drop table molecule_test
insert into molecule_test(a_id, a) values ('154654654'::float8, 'NC1C(O)C(O)C(P)C(F)C1Cl')
select a_id from molecule_test
select count(*) from molecule_test
create index molecule_test_idx on molecule_test using bingo_idx (a bingo.molecule)
explain select * from molecule_test where a @ ('NC1C(O)C(O)C(P)C(F)C1Cl', '')::bingo.sub
select count(*) from molecule_test where a @ ('CN1N(C(=O)C=C1C)C1=CC=CC=C1', '')::bingo.sub

drop table min_oci_test
create table min_oci_test(a_id serial, a text);
select * from min_oci_test
select bingo.importSDF('min_oci_test', 'a', '', 
'/home/tarquin/projects/indigo/bugs/postgres/min_oci_call_error.sdf');
create index min_oci_test_idx on min_oci_test using bingo_idx (a bingo.molecule)
explain select * from min_oci_test where a @ ('CN1N(C(=O)C=C1C)C1=CC=CC=C1', '')::bingo.sub

select * from btest where ctid = '(0, 90)'::tid
select * from molecule_test where a @ ('CCCCCNC(C)=O', 'TAU')::bingo.sub; 
select * from molecule_test where a @ ('CCCCCNC(C)=O', '')::bingo.sub; 
select count(*) from molecule_test where a @ (0.5, 1, 'CCCCCNC(C)=O', '')::bingo.sim

select * from pg_stat_activity;
select pg_cancel_backend(3859);
select kill(3859)
select bingo.getweight(a, ''), a from btest where a @ ('CC', '')::bingo.sub
select * from btest where a @ ('CC', '')::bingo.sub
drop index btest_idx
create index btest_idx on btest using bingo_idx (a bingo.molecule)

select * from btest where a @ ('GGGNC(=O)', '')::bingo.sub
select * from btest where a @ ('GGGCC', '')::bingo.exact
select bingo.getweight('CCC>>ССС', '')

select bingo._get_profiling_info()
select bingo._reset_profiling_info()
select bingo._print_profiling_info()


create table clean_public_mols (a serial, data text)
select bingo.importSdf('clean_public_mols','data','', '/home/tarquin/projects/indigo/bugs/postgres/clean_for_bingo.sdf')
select * from clean_public_mols limit 10
create index clean_public_mols_idx on clean_public_mols using bingo_idx (data bingo.molecule);
select a from clean_public_mols where data @ ('c1ccc2ccccc2c1', '' )::bingo.sub;1158
SELECT data FROM clean_public_mols WHERE ctid='(3760,5)'::tid
drop index clean_public_mols_idx
select * from chembl_test where data @ ('CCCCCNC(C)=O', '')::bingo.sub;
select * from chembl_test where ctid='(29324,5)'::tid


insert into btest(a)  (select data from chembl_test where ctid='(29324,5)'::tid)
create index btest_idx on btest using bingo_idx (a bingo.molecule)


select * from btest where a @ ('CCCCCNC(C)=O', '')::bingo.sub;
drop table btest

drop table chembl_test2
create table chembl_test2 (a int, data text)

insert into chembl_test2(a, data) select a, data from chembl_test where (a > 253000 and a < 254361 )
create index chembl_idx2 on chembl_test2 using bingo_idx (data bingo.molecule)
select * from chembl_test2 where data @ ('CCCCCNC(C)=O', '')::bingo.sub;
select * from chembl_test2 where data @ ('NC(CSC1CC(=O)N(CCC(O)=O)C1=O)CC=O', '')::bingo.sub;
select * from chembl_test where data @ ('NC(CSC1CC(=O)N(CCC(O)=O)C1=O)CC=O', '')::bingo.sub;


select insert_table('CCC', 129000)

select bingo.fingerprint('C1CCCC1', 'full')
select bingo.fingerprint('C1CCCC1', 'aa')
select bingo.smiles('AAA')
select bingo.rfingerprint('C1CCCC1>>C1CCC1', 'full')
select bingo.rfingerprint('C1CCCC1>>C1CCC1', 'aa')
select bingo.compactmolecule('C1CCCC1', false)
select bingo.compactreaction('C1CCCC1>>C1CCC1', false)

select bingo.inchi('C1CCCC1', '')
select bingo.inchi('C1CCCC1', '/DoNotAddH')
select bingo.inchi('C1CCCC1', '/AAA')
select bingo.inchikey('InChI=1S/C5/c1-2-4-5-3-1')
select bingo.inchikey('AAA')

select bingo.smiles(bingo.compactMolecule('CCC', false))
select bingo.smiles('CCC')
select bingo.cansmiles('CCC')
select bingo.cansmiles(bingo.compactMolecule('CCC', false))

explain select * from btest where a @ ('CC(=O)', '')::bingo.sub

update btest set b = bingo.compactMolecule(a, false)

explain select * from btest where b @ ('CC(=O)', '')::bingo.sub
create index btest_idx2 on btest using bingo_idx_b (b bingo.molecule)

select bingo.compactMolecule('NC1CCCC(C1)C1CCCCC1',false) @ ('CC1CCCCC1', '')::bingo.smarts

create table test(id serial primary key, data text);
select bingo.importsdf('btest', 'a', '', 'c:/_work/repo/indigo-git/_projects/pubchem_slice_10000000.smiles');

select * from test


create table chembl_test (a serial, data text)
select bingo.importsdf('chembl_test', 'data', '', '/home/tarquin/projects/bases/chembl_04.sdf');
create index chembl_idx on chembl_test using bingo_idx (data bingo.molecule)

select * from chembl_test where data@ ('*1*******1','')::bingo.sub limit 100;

explain select * from chembl_test where data@ ('CCCCCNC(C)=O','')::bingo.sub limit 100;
select count(*) from chembl_test

select bingo.smiles(data) from chembl_test where ctid='(336,12)'::tid
select data from chembl_test where ctid='(336,12)'::tid


insert into btest(a)  (select data from chembl_test where ctid='(336,12)'::tid)
select * from btest where a @ ('*1*******1','')::bingo.sub;
insert into btest(a) values('C12SC3N(C=CN=3)C=1C(C1=C(C=CC(O)=C1C2=O)Cl)=O');

create table chembl_test222 (a serial, data text)
select bingo.importsmiles('chembl_test222', 'data', '', '/home/tarquin/projects/bases/test2.sdf');


﻿drop table test_pubchem_slice;
create table test_pubchem_slice(id serial, data text);
select bingo.importsdf('public.test_pubchem_slice', 'data', '', '/home/tarquin/projects/bases/pubchem_slice_10000000.smiles');
create index test_pubchem_slice_idx on test_pubchem_slice using bingo_idx (data bingo.molecule);
select id from test_pubchem_slice where data @ ('C1(C=CC(S)=CC=1)/C=C/C(OC)=O', '') :: bingo.sub;
select id from test_pubchem_slice limit 10

drop table sch_test
create table sch_test(a serial, b text)
select bingo.importsdf('public.sch_test', 'b', '', '/home/tarquin/projects/bases/sch_50k.rdf');
select count(*) from sch_test
create index sch_test_idx on sch_test using bingo_idx (b bingo.reaction)
select * from sch_test where b @ ('*1***C**1>>', '')::bingo.rsub limit 100
drop index sch_test_idx

update bingo.bingo_config set cvalue='20' where cname='NTHREADS'

drop table pubchem_slice
create table pubchem_slice(a text, b serial)
select bingo.importsmiles('public.pubchem_slice', 'a', '', '/home/tarquin/projects/bases/pubchem_slice_100000.smiles');

create index pubchem_slice_idx on pubchem_slice using bingo_idx(data bingo.molecule)

select * from pubchem_slice

explain select * from (select a.b as regno, b.b as dup from btest a, btest b 
where a.a @ (b.a, '')::bingo.exact order by a.b) as duplicate where regno <> dup;

explain select * from (select a.b as regno, b.b as dup from btest a btest b 
where a.a @ (b.a, '')::bingo.exact order by a.b) as duplicate where regno <> dup;

explain select * from (select x.b as regno, y.b as dup from btest2 x, btest2 y 
where x.a @ (y.a, '')::bingo.exact order by x.b) as duplicate where regno <> dup;

explain select btest.b, z.dup  from (select x.a as a1, y.a as a2, x.b as regno, y.b as dup from btest x cross join btest y) as z 
join btest on btest.a @ (z.a1, '')::bingo.exact and btest.b <> z.regno and btest.b <> z.dup order by z.dup


drop table btest_init
create table btest_init(a text, b serial);
insert into btest_init(a) (select a from pubchem_slice limit 200)


select * from (select x.b as regno, y.b as dup from btest_init x, btest_init y 
where x.a @ (y.a, '')::bingo.exact order by x.b) as duplicate where regno <> dup;

select b from btest_init

insert into btest_init(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest_init(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest_init(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest_init(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest_init(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');


drop table btest2;
create table btest2(a text, b serial);
insert into btest2(a,b) (select a,b from btest_init);

drop table btest;
create table btest(a text, b serial);
insert into btest(a,b) (select a,b from btest_init);


create index btest_idx on btest using bingo_idx (a bingo.molecule)
drop index btest_idx

explain select * from (select x.b as regno, y.b as dup from btest x, btest y 
where x.a @ (y.a, '')::bingo.exact order by x.b) as duplicate where regno <> dup;



drop table btest3;
create table btest3(regno integer, dup integer);
insert into btest3(b)(select b from btest_init);
insert into btest3(b) values(100);
insert into btest3(b) values(101);
insert into btest3(b) values(102);
insert into btest3(b) values(103);
insert into btest3(b) values(104);
select * from btest3
create index btest3_idx on btest3 using hash(b)

explain select * from (select x.b as regno, y.b as dup from btest3 x, btest3 y 
where x.b=y.b order by x.b) as duplicate ;

select bingo._print_profiling_info()

select id, 1 from public.exact_m_m_t where data @ ((select data from public.exact_m_m_q where id = 44), '') :: bingo.exact order by id asc


insert into btest(a) values('C[H].C[2H].C[3H].[He]C.[Li]C.[Be]C.BC.CC.CN.CO.CF.C[Ne].C[Na].C[Mg].C[AlH2].C[SiH3].CP.CS.CCl.C[Ar].C[K].C[Ca].C[Sc].C[Ti].C[V].C[Cr].C[Mn].C[Fe].C[Co].C[Ni].C[Cu].C[Zn].C[GaH2].C[GeH3].C[AsH2].C[SeH].CBr.C[Kr].C[Rb].C[Sr].C[Y].C[Zr].C[Nb].C[Mo].C[Tc].C[Ru].C[Rh].C[Pd].C[Ag].C[Cd].C[InH2].C[SnH3].C[SbH2].C[TeH].CI.C[Xe].C[Cs].C[Ba].C[La].C[Ce].C[Pr].C[Nd].C[Pm].C[Sm].C[Eu].C[Gd].C[Tb].C[Dy].C[Ho].C[Er].C[Tm].C[Yb].C[Lu].C[Hf].C[Ta].C[W].C[Re].C[Os].C[Ir].C[Pt].C[Au].C[Hg].C[Tl].C[PbH].C[BiH2].C[PoH].C[At].C[Rn].C[Fr].C[Ra].C[Ac].C[Th].C[Pa].C[U].C[Np].C[Pu].C[Am].C[Cm].C[Bk].C[Cf].C[Es].C[Fm].C[Md].C[No].C[Lr].C[Rf]');

explain select bingo.smiles(a) from btest where a @ ('CC', '-FRA')::bingo.exact

select 'C[H].C[2H].C[3H].[He]C.[Li]C.[Be]C.BC.CC.CN.CO.CF.C[Ne].C[Na].C[Mg].C[AlH2].C[SiH3].CP.CS.CCl.C[Ar].C[K].C[Ca].C[Sc].C[Ti].C[V].C[Cr].C[Mn].C[Fe].C[Co].C[Ni].C[Cu].C[Zn].C[GaH2].C[GeH3].C[AsH2].C[SeH].CBr.C[Kr].C[Rb].C[Sr].C[Y].C[Zr].C[Nb].C[Mo].C[Tc].C[Ru].C[Rh].C[Pd].C[Ag].C[Cd].C[InH2].C[SnH3].C[SbH2].C[TeH].CI.C[Xe].C[Cs].C[Ba].C[La].C[Ce].C[Pr].C[Nd].C[Pm].C[Sm].C[Eu].C[Gd].C[Tb].C[Dy].C[Ho].C[Er].C[Tm].C[Yb].C[Lu].C[Hf].C[Ta].C[W].C[Re].C[Os].C[Ir].C[Pt].C[Au].C[Hg].C[Tl].C[PbH].C[BiH2].C[PoH].C[At].C[Rn].C[Fr].C[Ra].C[Ac].C[Th].C[Pa].C[U].C[Np].C[Pu].C[Am].C[Cm].C[Bk].C[Cf].C[Es].C[Fm].C[Md].C[No].C[Lr].C[Rf]' @ ('CC', '-FRA')::bingo.exact

select id, 1 from public.exact_m_m_t where data @ ((select data from public.exact_m_m_q where id = 71), 'ELE STE') :: bingo.exact order by id asc

select id, 1 from public.exact_m_m_t where data @ ((select data from public.exact_m_m_q where id = 70), 'ELE STE') :: bingo.exact order by id asc

select id, 1 from public.exact_m_m_t where data @ ((select data from public.exact_m_m_q where id = 71), 'ELE STE') :: bingo.exact order by id asc


select count(id) from public.similarity_m_m_t where data @ (0.9, 0.99, 'S', 'tanimoto') :: bingo.sim 
select count(id) from public.similarity_m_m_t where bingo.matchSim(data, (0.5, 0.99, 'S', 'tanimoto') :: bingo.sim) order by id asc

drop table join_m_m_t;
create table join_m_m_t as (select * from checkmolecule_m_m_q where bingo.checkmolecule(data) is null);
create table join_m_m_t as (select * from checkmolecule_m_m_q where bingo.checkmolecule(data) is null limit 700);
create table join_m_m_t as (select * from checkmolecule_m_m_q where bingo.checkmolecule(data) is null limit 25 offset 700);
create index join_idx on join_m_m_t using bingo_idx (data bingo.molecule)
drop index join_idx


select * from join_m_m_t
explain select * from (select x.id as regno, y.id as dup from join_m_m_t x, join_m_m_t y 
where x.data @ (y.data, '') :: bingo.exact order by x.id) as duplicate where regno <> dup

select * from (select x.id as regno, y.id as dup from join_m_m_t x, join_m_m_t y 
where x.data @ (y.data, '') :: bingo.sub order by x.id) as duplicate where regno <> dup

select * from join_idx_shadow
select * from join_idx_shadow_hash

select bingo._print_profiling_info()
select bingo._reset_profiling_info()

explain select sh.b_id from join_idx_shadow sh, join_idx_shadow_hash t0, join_idx_shadow_hash t1, join_idx_shadow_hash t2, 
join_idx_shadow_hash t3, join_idx_shadow_hash t4, join_idx_shadow_hash t5, join_idx_shadow_hash t6, 
join_idx_shadow_hash t7, join_idx_shadow_hash t8, join_idx_shadow_hash t9 where 
t4.ex_hash = 1082584219 AND t5.ex_hash = 1412634575 AND t6.ex_hash = -1302344237 AND t7.ex_hash = -223495129 AND 
t8.ex_hash = 213635681 AND t9.ex_hash = -1082012957 AND t0.f_count = 3 AND t1.f_count = 1 AND t2.f_count = 1 AND 
t0.ex_hash = 771813063 AND t1.ex_hash = -797207101 AND t2.ex_hash = 1174076319 AND t3.ex_hash = -1571572743 AND
sh.b_id = t0.b_id AND t0.b_id = t1.b_id AND t1.b_id = t2.b_id AND t2.b_id = t3.b_id AND t3.b_id = t4.b_id AND 
t4.b_id = t5.b_id AND t5.b_id = t6.b_id AND t6.b_id = t7.b_id AND t7.b_id = t8.b_id AND t8.b_id = t9.b_id AND 
t3.f_count = 1 AND t4.f_count = 1 AND t5.f_count = 3 AND t6.f_count = 1 AND t7.f_count = 1 AND t8.f_count = 1 AND 
t9.f_count = 1 AND sh.fragments = 14

create index j_index1 on join_idx_shadow_hash (b_id, ex_hash)
create index j_index2 on join_idx_shadow_hash (b_id, f_count)
create index j_index3 on join_idx_shadow_hash (b_id, ex_hash, f_count)
create index j_index4 on join_idx_shadow_hash ( ex_hash, f_count)

create index j_index5 on join_idx_shadow_hash using hash (f_count)
create index j_index6 on join_idx_shadow_hash (b_id)

drop index j_index1
drop index j_index2
drop index j_index3
drop index j_index4
drop index j_index5
drop index j_index6

select * from join_m_m_t where ctid='(0,23)'::tid

update bingo.bingo_config set cvalue = '1000' where cname = 'TIMEOUT'
select * from bingo.bingo_config

explain select * from btest where a @ ('C=O', '')::bingo.sub
explain select * from btest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo.exact

grant usage on schema bingo to test;
grant select on table bingo.bingo_config to test;
grant select on table bingo.bingo_tau_config to test;
grant select on table btest to test;
grant select on table btest_idx_shadow to test
grant select on table btest_idx_shadow_hash to test
revoke select on table btest_idx_shadow from test
revoke select on table btest_idx_shadow_hash from test

explain select * from btest where a @ ('CC(=O)', '')::bingo.sub
explain select * from btest where a @ ('CC(=O)', '')::bingo.sub and bingo.getMass(a) > 100 and bingo.getMass(a)<300

select * from btest where a @ ('CC(=O)', 'B_ID 1')::bingo.sub

drop table test_pubchem_10m
create table test_pubchem_10m(m_id serial, a text);
create table test_pubchem_1m(m_id serial, a text);
select bingo.importsmiles('test_pubchem_10m', 'a', '', 'c:/_work/Indigo/bases/pubchem_slice_10m.smiles');

create index pb10m_idx on test_pubchem_10m using bingo_idx (a bingo.molecule)
insert into test_pubchem_1m select * from test_pubchem_10m limit 1000000
select count(*) from test_pubchem_1m
create index pb1m_idx on test_pubchem_1m using bingo_idx (a bingo.molecule)
select count(*) from test_pubchem_1m where a @ ('CN1N(C(=O)C=C1C)C1=CC=CC=C1', '')::bingo.sub
select count(*) from test_pubchem_1m where a @ ('CN1N(C(=O)C=C1C)C1=CC=CC=C1', 'B_ID 1 B_COUNT 3')::bingo.sub
--2.5 sec (7.5)
select count(*) from test_pubchem_1m where a @ ('C(C)COC=O', '')::bingo.sub
select count(*) from test_pubchem_1m where a @ ('C(C)COC=O', 'B_ID 1 B_COUNT 3')::bingo.sub
select * from test_pubchem_1m where a @ ('C(C)COC=O', '')::bingo.sub limit 10000

select count(*) from test_pubchem_1m where a @ ('C1C(C1)CCO', '')::bingo.sub limit 10000
select count(*) from test_pubchem_1m where a @ ('CN1N(C(=O)C=C1C)C1=CC=CC=C1', 'B_ID 6')::bingo.sub

select count(*) from test_pubchem_1m where a @ (0.9, 1, 'CN1N(C(=O)C=C1C)C1=CC=CC=C1', 'B_ID 1 B_COUNT 3')::bingo.sim

select bingo.getstructurescount('btest_idx')
select bingo.getblockcount('btest_idx')
select bingo.precacheDatabase('btest_idx', 'KB')
select bingo._precache_database('pb1m_idx'::regclass::oid, 'MB')

drop function bingo._precache_database(oid, text)

CREATE OR REPLACE FUNCTION bingo._precache_database(oid, text)
RETURNS text
AS 'c:/_work/repo/indigo-git/_projects/pg_lib92_x64/lib/bingo_postgres'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION insert_table(text, integer) RETURNS void AS $$
declare idx integer;
begin
 idx := 0;
 LOOP
    INSERT INTO btest(a) values ($1);
    idx := idx + 1;
    EXIT WHEN idx > $2;
    
 END LOOP;
end;
$$ LANGUAGE 'plpgsql' ;

select insert_table('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3', 64000)

insert into bingo.bingo_config(cname, cvalue) values ('zero_unknown_aromatic_hydrogens', '0');
"CCCCCCCCCCCC.CCCCCCCCCCC.CCCCCCCC.CCCCCCCCCCC"
"C1=CC(O)=CC(O)=C1""C1=CC=C(NCCCC)C=C1O"
select * from chemistry.compound
explain select compound_id,smiles from chemistry.compound where structure @ ('C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1', '')::bingo.sub
select compound_id,smiles from chemistry.compound where structure @ ('c1ccccc1', '')::bingo.exact
select * from chemistry.compound where compound_id=1527
vacuum full chemistry.compound
drop index chemistry.bingo_compound_index
CREATE INDEX bingo_compound_index ON chemistry.compound USING bingo_idx (structure bingo.molecule);



drop table qc.cmp_test
drop index qc.cmp_test_idx
select compound_id,smiles,structure into qc.cmp_test from chemistry.compound
select * from qc.cmp_test where ctid='(103,7)'::tid
create index cmp_test_idx on qc.cmp_test using bingo_idx (structure bingo.molecule)
explain select compound_id,smiles from qc.cmp_test where structure @ ('C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1.C1CCCCC1', '')::bingo.exact
select compound_id,smiles from qc.cmp_test where structure @ ('c1ccccc1', '')::bingo.exact
select * from qc.cmp_test where compound_id=1527
vacuum full qc.cmp_test

select compound_id,smiles from chemistry.compound where structure @ ('c1ccccc1', '')::bingo.exact


explain select compound_id,smiles from qc.cmp_test where structure @ ('c1ccccc1', '')::bingo.exact


select bingo.importsdf('btest', 'a', '', '/home/tarquin/Downloads/amb9678132.sdf');
insert into btest(a) values('[O-][N+](=O)c1cc([N+](=O)[O-])c2c(c1)[n+]1nc3c([n-]1n2)cc(cc3[N+](=O)[O-])[N+](=O)[O-]');

drop table b_st
create table b_st (i int, mol text, name text)
select bingo.importsdf('b_st', 'mol', 'ID i, name name', '/home/tarquin/Downloads/Test stereo 2.sdf');
select * from b_st

select distinct(b.i), b.name from b_st as a
inner join b_st b on b.mol @ (a.mol, 'ALL')::bingo.exact
where a.i=5

select * from b_st


drop table b_ster
create table b_ster (i int, mol text, name text)
select bingo.importsdf('b_ster', 'mol', 'ID i, name name', '/opt/projects/2/INDSP-273/5.sdf');
select bingo.importsdf('b_ster', 'mol', 'ID i, name name', '/opt/projects/2/INDSP-273/6.sdf');
select bingo.importsdf('b_ster', 'mol', 'ID i, name name', '/opt/projects/2/INDSP-273/50.sdf');
select bingo.importsdf('b_ster', 'mol', 'ID i, name name', '/opt/projects/2/INDSP-273/60.sdf');

select * from b_ster

select distinct(b.i), b.name from b_ster as a
inner join b_ster b on b.mol @ (a.mol, 'ALL')::bingo.exact
where a.i=6


select distinct(b.i), b.name from b_ster as a
inner join b_ster b on b.mol @ (a.mol, 'ALL')::bingo.exact
where a.i=5

select distinct(b.i), b.name from b_ster as a
inner join b_ster b on b.mol @ (a.mol, 'NONE')::bingo.exact
where a.i=5


insert into btest(a) values('CN1C2C=C(/C(=N/NC(=O)N)/C)C=CC=2SC2C(P)=C(C)C(CCC)=ClC1=2');
insert into btest(a) values ('N1C=C(CC(C)CC)C(=O)N(C)C1=O')

sl

select * from btest where a @ ('CN1C2C=C(/C(=N/NC(=O)N)/C)C=CC=2SC2C(P)=C(C)C(CCC)=ClC1=2', '')::bingo.sub
d


select * from btest where a @ ('N1C=C(CC(C)CC)C(=O)N(C)C1=O', '')::bingo.sub
select * from btest where a @ ('N1C=C(CC(C)CC)C(=O)N(C)C1=O', '')::bingo.exact


drop table b_fing
create table b_fing (a text, b serial);
create index b_fing_idx on b_fing using bingo_idx (a bingo.molecule)


select bingo.importSmiles('b_fing', 'a', '', '/home/tarquin/projects/indigo-git/tests/api.as/data/ind_692.smiles');

select distinct(b.b), b.a from b_fing as a
inner join b_fing b on b.a @ (a.a, 'NONE')::bingo.exact
where a.b=5

explain select distinct(b.b), b.a from b_fing as a
inner join b_fing b on b.a @ (a.a, '')::bingo.sub
where a.b=38

select * from b_fing where b=38

select '[C@]123CCC[C@H]1C[C@@H]([C@H]([2H])[C@@H]2[2H])C3' @ ('[C@]123CCC[C@H]1C[C@@H]([C@H]([2H])[C@@H]2[2H])C3', '')::bingo.sub

select 'OC(CCCCCC=C)C=CC#CC#CC(O)C=C' @ ('OC(CCCCCC=C)C=CC#CC#CC(O)C=C', '')::bingo.sub

drop table pubchem_1m
create table pubchem_1m (a text, b serial);
select bingo.importSmiles('pubchem_1m', 'a', '', '/opt/_work/Indigo/bases/pubchem_slice_1m.smiles');
create index pubchem_1m_idx on pubchem_1m using bingo_idx (a bingo.molecule) with (NTHREADS=32)

select distinct(b.b), b.a from pubchem_1m as a
inner join pubchem_1m b on b.a @ (a.a, '')::bingo.sub
where a.b=1903

select * from pubchem_1m where b=1903

select 'I(=I)I' @ ('I(=I)I', '')::bingo.sub



create table pubchem_10m (a text, b serial);

select 

select bingo.importSdf('pubchem_1m', 'a', '', '/opt/_work/Indigo/bases/pubchem_1M_sd.gz');

create index pubchem_1m_idx on pubchem_1m using bingo_idx (a bingo.molecule) 

select * from pubchem_1m where a @ ('OC1=CC=CC=C1-O-O', '')::bingo.sub limit 10
select count(*) from pubchem_1m where a @ ('OC1=CC=CC=C1-O-O', '')::bingo.sub
select bingo._reset_profiling_info()
select bingo._print_profiling_info()

select count(*) from pubchem_1m where a @ ('c1ccccc1', '')::bingo.sub

create table test_raw (a text, b serial);
select bingo.importSdf('test_raw', 'a', '', '/opt/_work/Indigo/bases/pubchem_10k.sd.gz')
create index test_raw_idx on test_raw using bingo_idx (a bingo.molecule)

create table test_gz (m bytea, p text, b serial);
select count(*) from test_gz 
create index test_gz_idx on test_gz using bingo_idx (m bingo.bmolecule)




select b, bingo.smiles(m.m)
from
test_gz m
where
m.m @ ('[CX3](=[OX1])[OX2H]','')::bingo.smarts AND
m.m @ ('C=C','')::bingo.smarts

create table test_34 (a text, b serial);
insert into test_34(a) values('CCCCCCCCCC@HCC(O)=O');
insert into test_34(a) values('CCCCCCCCC@HCCC(O)=O');
insert into test_34(a) values('CC(CO)C(O)=O');
insert into test_34(a) values('OC(=O)Cc1ccccc1');
insert into test_34(a) values('Cc1ccccc1C(O)=O');
insert into test_34(a) values('Cc1cc(ccc1)C(O)=O');
insert into test_34(a) values('Cc1ccc(cc1)C(O)=O');

create index test_34_idx on test_34 using bingo_idx (a bingo.molecule)

drop index test_34_idx


explain 
select * from test_34 m where
m.a @ ('[CX3](=[OX1])[OX2H]','')::bingo.smarts AND
m.a @ ('C=C','')::bingo.smarts

bingo.matchsmarts(m.a, ('C=C',''))


create table test_100k (m bytea, p text, b serial);

select count(*) from test_100k
create index CONCURRENTLY test_100k_idx on test_100k using bingo_idx (m bingo.bmolecule)
drop index test_100k_idx

explain 
select b, bingo.smiles(m.m) from test_100k m where m.m @ ('Cc1ccc(cc1)C(O)=O','')::bingo.sub limit 30
select bingo._reset_profiling_info()
select bingo._print_profiling_info()


explain
select * from test_34 where a @ ('CC','')::bingo.sub 
drop 
create index test_34_idx on test_34 using bingo_idx (a bingo.molecule)

SELECT bingo.getIndexStructuresCount('btest_idx'::regclass::oid)


insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');

select bingo.getversion()

select * from bingo.bingo_config

UPDATE bingo.bingo_config set cvalue = 'ECFP6' where cname = 'SIMILARITY_TYPE'

UPDATE bingo.bingo_config set cvalue = 'SIM' where cname = 'SIMILARITY_TYPE'

UPDATE bingo.bingo_config set cvalue = '1' where cname = 'IGNORE_BAD_VALENCE'


select bingo.getSimilarity('Cc1sc2c(C(=N[C@@H](CC(=O)OC(C)(C)C)c3nnc(C)n23)c4ccc(Cl)cc4)c1C', 'O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C', '')

select bingo.getSimilarity('P(OCC(O)C(O)(CO)C)(O)(O)=O', 'O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C', '')


create index sim_idx on btest using bingo_idx (a bingo.molecule)


explain 

select a from btest where a @ (0,0.01, 'P(OCC(O)C(O)(CO)C)(O)(O)=O', '')::bingo.sim

select a, bingo.getSimilarity(a, 'Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12', '') from btest where a @ (0,0.5, 'Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12', '')::bingo.sim


drop index sim_idx

