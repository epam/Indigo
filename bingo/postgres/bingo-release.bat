rmdir /S /Q %1

mkdir %1\bin
mkdir %1\sql

copy ..\LICENSE.GPL %1\
copy README %1\
copy INSTALL %1\
copy %3 %1\bin\
copy bingo-pg-install.bat %1\
copy sql\%2\* %1\sql 

zip -r -9 %1.zip %1
