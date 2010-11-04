rmdir /S /Q %1

mkdir %1

copy ..\..\api\LICENSE.GPL %1\
copy %2\Release\indigo-deco.exe %1

zip -r -9 %1.zip %1
