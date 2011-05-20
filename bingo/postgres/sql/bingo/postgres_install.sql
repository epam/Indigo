CREATE SCHEMA bingo;

SET search_path = bingo;

CREATE OR REPLACE FUNCTION bingo_build(internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_insert(internal, internal, internal, internal, internal)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_beginscan(internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_gettuple(internal, internal)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_getbitmap(internal, internal)
RETURNS int8
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_rescan(internal, internal)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_endscan(internal)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_markpos(internal)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_restrpos(internal)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_bulkdelete(internal, internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_vacuumcleanup(internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_options(_text, bool)
RETURNS bytea
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingo_costestimate(internal, internal, internal, internal, internal, internal, internal, internal)
RETURNS void
AS 'MODULE_PATHNAME'
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
7,
7,
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


CREATE OR REPLACE FUNCTION bingoImportSDF(text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoImportRDF(text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION bingoGetIndexStructuresCount(oid)
RETURNS integer
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingoMassType(text, text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingoMass(text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_sub_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_smarts_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_exact_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_gross_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE bingo_sub AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_sub_sql(text, bingo_sub)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo.bingo_sub_internal($2.query_mol, $1, $2.query_options);
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

CREATE TYPE bingo_gross AS (sign text, query_mol text);

CREATE OR REPLACE FUNCTION bingo_gross_sql(text, bingo_gross)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo_gross_internal($2.sign, $2.query_mol, $1);
   END;
$$ LANGUAGE 'plpgsql';

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = bingo_sub,
        PROCEDURE = bingo_sub_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = bingo_exact,
        PROCEDURE = bingo_exact_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = bingo_smarts,
        PROCEDURE = bingo_smarts_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = bingo_gross,
        PROCEDURE = bingo_gross_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
--******************* MASS *******************
CREATE TYPE bingo_mass; 

CREATE OR REPLACE FUNCTION bingo_mass_in(cstring)
    RETURNS bingo_mass
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION bingo_mass_out(bingo_mass)
    RETURNS cstring
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE bingo_mass (
   internallength = variable, 
   input = bingo_mass_in,
   output = bingo_mass_out
);

CREATE OR REPLACE FUNCTION bingo_mass_great_internal(text, bingo_mass)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION bingo_mass_less_internal(text, bingo_mass)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;


CREATE OPERATOR public.< (
        LEFTARG = text,
        RIGHTARG = bingo_mass,
        PROCEDURE = bingo_mass_less_internal,
        COMMUTATOR = '<',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE OPERATOR public.> (
        LEFTARG = text,
        RIGHTARG = bingo_mass,
        PROCEDURE = bingo_mass_great_internal,
        COMMUTATOR = '>',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

--******************* SIMILARITY *******************

CREATE OR REPLACE FUNCTION bingoSim(text, text, text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE bingo_sim AS (min_bound real, max_bound real, query_mol text, query_options text);

CREATE OR REPLACE FUNCTION bingo_sim_internal(real, real, text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;


CREATE OR REPLACE FUNCTION bingo_sim_sql(text, bingo_sim)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo_sim_internal($2.min_bound, $2.max_bound, $2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = bingo_sim,
        PROCEDURE = bingo_sim_sql,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);


--**************************** BINGO OPERATOR CLASS *********************
CREATE OPERATOR CLASS bingo_ops
FOR TYPE text USING bingo_idx
AS
        OPERATOR        1       public.@ (text, bingo_sub),
        OPERATOR        2       public.@ (text, bingo_exact),
        OPERATOR        3       public.@ (text, bingo_smarts),
        OPERATOR        4       public.@ (text, bingo_gross),
        OPERATOR        5       public.< (text, bingo_mass),
        OPERATOR        6       public.> (text, bingo_mass),
        OPERATOR        7       public.@ (text, bingo_sim),
        FUNCTION	1	bingo_sub_sql(text, bingo_sub),
        FUNCTION	2	bingo_exact_sql(text, bingo_exact),
        FUNCTION	3	bingo_smarts_sql(text, bingo_smarts),
        FUNCTION	4	bingo_gross_sql(text, bingo_gross),
        FUNCTION	5	bingo_mass_less_internal(text, bingo_mass),
        FUNCTION	6	bingo_mass_great_internal(text, bingo_mass),
        FUNCTION	7	bingo_sim_sql(text, bingo_sim);

        



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

drop table if exists bingo_tau_config;
create table bingo_tau_config(rule_idx integer, tau_beg text, tau_end text);
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (1, 'N,O,P,S,As,Se,Sb,Te', 'N,O,P,S,As,Se,Sb,Te');
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (2, '0C', 'N,O,P,S');
insert into bingo_tau_config(rule_idx, tau_beg, tau_end) values (3, '1C', 'N,O');





