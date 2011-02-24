rd /S /Q %1

cd java
call compile.bat
cd ..\renderer\java
call compile.bat
cd ..\..\

mkdir %1

copy LICENSE.GPL %1\

copy java\dist\indigo.jar %1\
copy renderer\java\dist\indigo-renderer.jar %1\
copy ..\common\jna\jna.jar %1\

zip -r -9 %1.zip %1
