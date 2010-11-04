rmdir /S /Q %1

mkdir %1\assembly
mkdir %1\assembly\lib32
mkdir %1\assembly\lib64

copy LICENSE.GPL %1\
copy sqlserver\sql\bingo_create.sql %1\
copy sqlserver\sql\bingo_create_methods.sql %1\
copy sqlserver\sql\bingo_drop.sql %1\
copy sqlserver\sql\bingo_drop_methods.sql %1\
copy sqlserver\sql\bingo-sqlserver-install.bat %1\
copy sqlserver\sql\bingo-sqlserver-uninstall.bat %1\
copy sqlserver\sql\assembly\bingo-sqlserver.dll %1\assembly\
copy sqlserver\sql\assembly\lib32\bingo-core-c.dll %1\assembly\lib32\
copy sqlserver\sql\assembly\lib64\bingo-core-c.dll %1\assembly\lib64\

zip -r -9 %1.zip %1
