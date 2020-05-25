-- indigoservice user
create user indigoservice with password 'p@ssw0rd';
create schema indigoservice authorization indigoservice;
grant all on table pg_depend to indigoservice;
grant usage on schema bingo to indigoservice;
grant select on table bingo.bingo_config to indigoservice;
grant select on table bingo.bingo_tau_config to indigoservice;

-- data schema
create table indigoservice.library_metadata(
    library_id varchar(36) primary key,
    service_data jsonb,
    user_data jsonb,
    index_data jsonb);
grant all on table indigoservice.library_metadata to indigoservice;

create table indigoservice.users(
    user_id serial primary key,
    username varchar(50) not null,
    password varchar(100) not null,
    email varchar(100) not null,
    foreign_auth_provider varchar(10),
    foreign_auth_id integer,
    user_created timestamp default now());
grant all on table indigoservice.users to indigoservice;
grant all on sequence indigoservice.users_user_id_seq to indigoservice;

-- upload testing schema
drop schema if exists test_upload cascade;
create schema test_upload authorization indigoservice;
