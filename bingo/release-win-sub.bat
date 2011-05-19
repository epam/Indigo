rmdir /S /Q %1

mkdir %1\bin
mkdir %1\sql\bingo
mkdir %1\sql\system

copy LICENSE.GPL %1\
copy %2 %1\bin\
copy sql\dbcheck.sql %1\sql\
copy sql\bingo-install.bat %1\sql\
copy sql\bingo\alter_routines.sql %1\sql\bingo\
copy sql\bingo\alter_routines2.sql %1\sql\bingo\
copy sql\bingo\bingo_calls.sql %1\sql\bingo\
copy sql\bingo\bingo_package.sql %1\sql\bingo\
copy sql\bingo\bingo_context.sql %1\sql\bingo\
copy sql\bingo\bingo_config.sql %1\sql\bingo\
copy sql\bingo\makebingo.sql %1\sql\bingo\
copy sql\bingo\dropbingo.sql %1\sql\bingo\
copy sql\bingo\mango_calls.sql %1\sql\bingo\
copy sql\bingo\mango_package.sql %1\sql\bingo\
copy sql\bingo\mango_drop.sql %1\sql\bingo\
copy sql\bingo\mango_index.sql %1\sql\bingo\
copy sql\bingo\mango_indextype.sql %1\sql\bingo\
copy sql\bingo\mango_make.sql %1\sql\bingo\
copy sql\bingo\mango_stat.sql %1\sql\bingo\
copy sql\bingo\ringo_calls.sql %1\sql\bingo\
copy sql\bingo\ringo_drop.sql %1\sql\bingo\
copy sql\bingo\ringo_index.sql %1\sql\bingo\
copy sql\bingo\ringo_indextype.sql %1\sql\bingo\
copy sql\bingo\ringo_make.sql %1\sql\bingo\
copy sql\bingo\ringo_package.sql %1\sql\bingo\
copy sql\bingo\ringo_stat.sql %1\sql\bingo\
copy sql\system\bingo_init.sql %1\sql\system\

zip -r -9 %1.zip %1
