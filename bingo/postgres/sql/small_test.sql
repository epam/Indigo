drop index btest_idx;
create index btest_idx on btest using bingo_idx (a bingo_ops);
