create table test1(m bytea, p jsonb NOT NULL DEFAULT '{}')

select * from test_celgene

select elems->>'idx_k',elems->>'idx_v' from test_celgene,jsonb_array_elements(p) elems where elems->>'idx_k' like '%id%' and (elems->>'idx_v')::float = 976001 

explain analyze
select elems->>'org_k',elems->>'org_v' from test_celgene,jsonb_array_elements(p) elems where elems->>'idx_k' like '%mass%' and (elems->>'idx_v')::float > 300 limit 50 offset 150

select elems->>'idx_k',elems->>'idx_v' from test_celgene,jsonb_array_elements(p) elems where elems->>'idx_k' like '%id%' and jsonb_typeof(elems->'idx_v') = 'number' and (elems->>'idx_v')::float = 976001

select elems

drop index test_celgene_idx
create index test_celgene_idx on test_celgene  USING bingo_idx (m bingo.bmolecule)

select elems->>'org_k',elems->>'org_v' from test_celgene,jsonb_array_elements(p) elems limit 1


select elems->>'a' as property ,elems->>'b' as value  from test_celgene,jsonb_array_elements(p) elems where elems->>'x' like '%mass%' and jsonb_typeof(elems->'y') = 'number' and  (elems->>'y')::float > 300 limit 50 offset 150

select * from bingo.bingo_config
drop index test_celgene_idx
create index test_celgene_idx on test_celgene  USING bingo_idx (m bingo.bmolecule) with (IGNORE_STEREOCENTER_ERRORS=1,IGNORE_CISTRANS_ERRORS=1,FP_TAU_SIZE=0)



select count(*) from test
select * from test limit 10
drop table test;
create table test (id serial, m bytea, p jsonb);
select * from 

select * from test_indigo_upload