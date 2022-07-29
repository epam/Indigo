create schema indigoservice;

create table indigoservice.library_metadata
(
    library_id   varchar(36) primary key,
    service_data jsonb,
    user_data    jsonb,
    index_data   jsonb
);

create table indigoservice.users
(
    user_id               serial primary key,
    username              varchar(50)  not null,
    password              varchar(100) not null,
    email                 varchar(100) not null,
    foreign_auth_provider varchar(10),
    foreign_auth_id       integer,
    user_created          timestamp default now()
);

-- upload testing schema
drop schema if exists test_upload cascade;
create schema test_upload;
