CREATE OR REPLACE FUNCTION bingo_build(internal, internal, internal)
RETURNS internal
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_insert(internal, internal, internal, internal, internal)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_beginscan(internal, internal, internal)
RETURNS internal
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_gettuple(internal, internal)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_getbitmap(internal, internal)
RETURNS int8
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_rescan(internal, internal)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_endscan(internal)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_markpos(internal)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_restrpos(internal)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_bulkdelete(internal, internal, internal, internal)
RETURNS internal
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_vacuumcleanup(internal, internal)
RETURNS internal
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_options(_text, bool)
RETURNS bytea
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_costestimate(internal, internal, internal, internal, internal, internal, internal, internal)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

--delete from pg_am where amname='bingo_idx'

INSERT INTO pg_am(
amname,
aminsert,
ambeginscan,
amgettuple,
amgetbitmap,
amrescan,
amendscan,
ammarkpos,
amrestrpos,
ambuild,
ambulkdelete,
amvacuumcleanup,
amcostestimate,
amoptions,
amstrategies,
amsupport,
amcanorder,
amcanbackward,
amcanunique,
amcanmulticol,
amoptionalkey,
amindexnulls,
amsearchnulls,
amstorage,
amclusterable,
amkeytype
)
VALUES (
'bingo_idx',
'bingo_insert(internal, internal, internal, internal, internal)'::regprocedure::oid,
'bingo_beginscan(internal, internal, internal)'::regprocedure::oid,
'bingo_gettuple(internal, internal)'::regprocedure::oid,
'bingo_getbitmap(internal, internal)'::regprocedure::oid,
'bingo_rescan(internal, internal)'::regprocedure::oid,
'bingo_endscan(internal)'::regprocedure::oid,
'bingo_markpos(internal)'::regprocedure::oid,
'bingo_restrpos(internal)'::regprocedure::oid,
'bingo_build(internal, internal, internal)'::regprocedure::oid,
'bingo_bulkdelete(internal, internal, internal, internal)'::regprocedure::oid,
'bingo_vacuumcleanup(internal, internal)'::regprocedure::oid,
'bingo_costestimate(internal, internal, internal, internal, internal, internal, internal, internal)'::regprocedure::oid,
'bingo_options(_text, bool)'::regprocedure::oid,
3,
3,
false,
true,
false,
false,
false,
false,
false,
false,
false,
23
);

--CREATE OR REPLACE FUNCTION bingo_sim_op(text,text)
--RETURNS bool
--AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
--LANGUAGE C STRICT STABLE;

--DROP OPERATOR IF EXISTS @ (text, text);

--CREATE OPERATOR @ (
--        LEFTARG = text,
--        RIGHTARG = text,
--        PROCEDURE = bingo_sim_op,
--        COMMUTATOR = '@',
--        RESTRICT = contsel,
--        JOIN = contjoinsel
--);


CREATE OR REPLACE FUNCTION bingoImportSDF_begin(text)
RETURNS bigint
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoImportSDF_end(bigint)
RETURNS void
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoimportsdf_next(bigint)
RETURNS text
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoimportsdf_hasnext(bigint)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoImportSDF_sql(text, text)
RETURNS boolean AS $$
   DECLARE
      sc bigint;
   BEGIN
	sc:= bingoImportSDF_begin($2);
	LOOP
	  EXIT WHEN NOT bingoImportSDF_hasnext(sc);
	  execute 'insert into ' || $1 || ' values(bingoimportsdf_next('|| sc ||'))';
	END LOOP;
	perform bingoImportSDF_end(sc);
	RETURN true;
   END;
$$ LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION bingoGetIndexStructuresCount(oid)
RETURNS integer
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_sub_internal(text, text, text)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_smarts_internal(text, text, text)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_exact_internal(text, text, text)
RETURNS boolean
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_sim_internal(text, text, text)
RETURNS real
AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
LANGUAGE C STRICT IMMUTABLE;

--CREATE OR REPLACE FUNCTION bingo_rsub_internal(text, text, text)
--RETURNS boolean
--AS '/home/tarquin/projects/indigo/postgres-git/bin/bingo_postgres_release'
--LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE bingo_sub AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_sub_sql(text, bingo_sub)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo_sub_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE bingo_exact AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_exact_sql(text, bingo_exact)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo_exact_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE bingo_smarts AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_smarts_sql(text, bingo_smarts)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo_smarts_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE bingo_sim AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_sim_sql(text, bingo_sim)
RETURNS real AS $$
   BEGIN
	RETURN bingo_sim_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE OPERATOR @ (
        LEFTARG = text,
        RIGHTARG = bingo_sub,
        PROCEDURE = bingo_sub_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR @ (
        LEFTARG = text,
        RIGHTARG = bingo_exact,
        PROCEDURE = bingo_exact_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR @ (
        LEFTARG = text,
        RIGHTARG = bingo_smarts,
        PROCEDURE = bingo_smarts_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE OPERATOR @ (
        LEFTARG = text,
        RIGHTARG = bingo_sim,
        PROCEDURE = bingo_sim_sql,
        COMMUTATOR = '@'
);

CREATE OPERATOR CLASS bingo_ops
FOR TYPE text USING bingo_idx
AS
        OPERATOR        1       @ (text, bingo_sub),
        OPERATOR        2       @ (text, bingo_exact),
        OPERATOR        3       @ (text, bingo_smarts),
        FUNCTION	1	bingo_sub_sql(text, bingo_sub),
        FUNCTION	2	bingo_exact_sql(text, bingo_exact),
        FUNCTION	3	bingo_smarts_sql(text, bingo_smarts);
        



--CREATE OPERATOR CLASS bingo_sim_ops
--FOR TYPE text USING bingo_idx
--AS
--        OPERATOR        1       @ (text, text),
--        FUNCTION	1	bingo_sim_op(text, text);

drop table if exists bingo_config;
create table bingo_config(cname varchar(255), cvalue varchar(255));
insert into bingo_config(cname, cvalue) values ('TREAT_X_AS_PSEUDOATOM', '0');
insert into bingo_config(cname, cvalue) values ('IGNORE_CLOSING_BOND_DIRECTION_MISMATCH', '0');
insert into bingo_config(cname, cvalue) values ('FP_ORD_SIZE', '25');
insert into bingo_config(cname, cvalue) values ('FP_ANY_SIZE', '15');
insert into bingo_config(cname, cvalue) values ('FP_TAU_SIZE', '10');
insert into bingo_config(cname, cvalue) values ('FP_SIM_SIZE', '8');
insert into bingo_config(cname, cvalue) values ('SUB_SCREENING_MAX_BITS', '8');
insert into bingo_config(cname, cvalue) values ('SIM_SCREENING_PASS_MARK', '128');

create table bingo_tau_config(rule_idx integer, tau_beg text, tau_end text);
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (1, 'N,O,P,S,As,Se,Sb,Te', 'N,O,P,S,As,Se,Sb,Te');
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (2, '0C', 'N,O,P,S');
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (3, '1C', 'N,O');


create table btest(a text);
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3C');
insert into btest(a) values('NC(=O)c4ccc3n(Cc2ccc1ccccc1c2)c(=O)c(=O)c3c4');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cc(O)cc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('CNC(=O)c2ccc1n(C)c(=O)c(=O)c1c2');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(N(=O)=O)ccc12');
insert into btest(a) values('Cn1c(=O)c(=O)c2cc(C(S)=N)ccc12');
insert into btest(a) values('NC(=O)c3ccc2n(Cc1cccc(O)c1)c(=O)c(=O)c2c3');
insert into btest(a) values('Cc3ccc(Cn1c(=O)c(=O)c2cc(C(N)=O)ccc12)cc3');