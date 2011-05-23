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


CREATE OR REPLACE FUNCTION importSDF(text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION importRDF(text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT VOLATILE;

CREATE OR REPLACE FUNCTION getIndexStructuresCount(oid)
RETURNS integer
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION getWeight(text, text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION getMass(text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION _sub_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION _smarts_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION _exact_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION _gross_internal(text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE sub AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION matchSub(text, sub)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo._sub_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE exact AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION matchExact(text, exact)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo._exact_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE smarts AS (query_mol text, query_options text);

CREATE OR REPLACE FUNCTION matchSmarts(text, smarts)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo._smarts_internal($2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE TYPE gross AS (sign text, query_mol text);

CREATE OR REPLACE FUNCTION matchGross(text, gross)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo._gross_internal($2.sign, $2.query_mol, $1);
   END;
$$ LANGUAGE 'plpgsql';

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = sub,
        PROCEDURE = matchSub,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = exact,
        PROCEDURE = matchExact,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = smarts,
        PROCEDURE = matchSmarts,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = gross,
        PROCEDURE = matchGross,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);
--******************* MASS *******************
CREATE TYPE mass;

CREATE OR REPLACE FUNCTION _mass_in(cstring)
    RETURNS mass
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION _mass_out(mass)
    RETURNS cstring
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE mass (
   internallength = variable, 
   input = _mass_in,
   output = _mass_out
);

CREATE OR REPLACE FUNCTION _match_mass_great(text, mass)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION _match_mass_less(text, mass)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;


CREATE OPERATOR public.< (
        LEFTARG = text,
        RIGHTARG = mass,
        PROCEDURE = _match_mass_less,
        COMMUTATOR = '<',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

CREATE OPERATOR public.> (
        LEFTARG = text,
        RIGHTARG = mass,
        PROCEDURE = _match_mass_great,
        COMMUTATOR = '>',
        RESTRICT = contsel,
        JOIN = contjoinsel
);

--******************* SIMILARITY *******************

CREATE OR REPLACE FUNCTION getSimilarity(text, text, text)
RETURNS real
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE sim AS (min_bound real, max_bound real, query_mol text, query_options text);

CREATE OR REPLACE FUNCTION _sim_internal(real, real, text, text, text)
RETURNS boolean
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;


CREATE OR REPLACE FUNCTION matchSim(text, sim)
RETURNS boolean AS $$
   BEGIN
	RETURN bingo._sim_internal($2.min_bound, $2.max_bound, $2.query_mol, $1, $2.query_options);
   END;
$$ LANGUAGE 'plpgsql';

CREATE OPERATOR public.@ (
        LEFTARG = text,
        RIGHTARG = sim,
        PROCEDURE = matchSim,
        COMMUTATOR = '@',
        RESTRICT = contsel,
        JOIN = contjoinsel
);


--**************************** BINGO OPERATOR CLASS *********************
CREATE OPERATOR CLASS ops
FOR TYPE text USING bingo_idx
AS
        OPERATOR        1       public.@ (text, sub),
        OPERATOR        2       public.@ (text, exact),
        OPERATOR        3       public.@ (text, smarts),
        OPERATOR        4       public.@ (text, gross),
        OPERATOR        5       public.< (text, mass),
        OPERATOR        6       public.> (text, mass),
        OPERATOR        7       public.@ (text, sim),
        FUNCTION	1	matchSub(text, sub),
        FUNCTION	2	matchExact(text, exact),
        FUNCTION	3	matchSmarts(text, smarts),
        FUNCTION	4	matchGross(text, gross),
        FUNCTION	5	_match_mass_less(text, mass),
        FUNCTION	6	_match_mass_great(text, mass),
        FUNCTION	7	matchSim(text, sim);

        



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





