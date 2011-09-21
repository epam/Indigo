rmdir /S /Q %1

mkdir %1\bin

copy ..\LICENSE.GPL %1\
copy README %1\
copy INSTALL %1\
copy %2 %1\bin\
copy bingo-pg-install.bat %1\
robocopy sql %1\sql /mir

zip -r -9 %1.zip %1
