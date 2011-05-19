select 'CC' @ ('CC', '')::bingo_sub
select 'CC' @ ('CC', '')::bingo_smarts
select 'NC1=CC=CC(=C1)C1=CC=CC=C1' @ ('=', 'C12H11N')::bingo_gross

select * from btest where a @ ('CC(=O)', '')::bingo_sub
select * from btest where a @ ('CCC', '')::bingo_sub
select * from btest where a @ ('OC1=CC=CC=C1', '')::bingo_sub

select * from btest where a @ ('CC(=O)', '')::bingo_exact
select * from btest where a @ ('CCC', '')::bingo_exact
select * from btest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo_exact
select * from btest where a @ ('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', 'TAU')::bingo_exact

explain select a, bingoSim(a, 'Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '') from btest 
where a @ (0.9, 1, 'Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C', '')::bingo_sim

select bingoSim('CCCCS(=N)(=O)CC[C@@H](N)C(O)=O', a, '') from test_table

select * from btest where a @ ('>=', 'C3')::bingo_gross

insert into btest(a) values('CCC');

vacuum btest

select 'CC' @ 'CC'
select ('CC', '')::molquery @ 'CC'
drop table btest_idx_shadow

select * from btest

delete from btest where a='CCC'
truncate table btest
 
select * from bingo_config
drop index btest_idx;
create index btest_idx on btest using bingo_idx (a bingo_ops)
drop table btest_idx_shadow
select * from btest_idx_shadow
select * from btest_idx_shadow_hash
create index btest_idx on btest using bingo_idx (a bingo_ops) with ( "treatx-as-pseudoatom=5", FP_SIM_SIZE=4);

explain select * from btest where a @ ('S', '')::bingo_sub
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

select 'bingo_insert(internal, internal, internal, internal, internal)'::regprocedure::oid

select oid from pg_proc where proname='bingo_insert'

select * from pg_am where amname in ('hash', 'bingo_idx')




create table btest(a text);

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
drop table btest




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



insert into test_table(a) values(null)
truncate table btest
select bingoImportSDF_sql('btest(a)', '/home/tarquin/projects/bases/acd2d_symyx.sdf')


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
