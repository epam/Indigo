rmdir /S /Q %1

mkdir %1\src

copy LICENSE.GPL %1\
copy cs\*.cs %1\src\
copy cs\Indigo.snk %1\src\
copy renderer\cs\indigo-renderer.snk %1\src\
copy renderer\cs\*.cs %1\src\
copy cs\bin\Release\indigo-cs.dll %1\
copy renderer\cs\bin\Release\indigo-renderer-cs.dll %1\

zip -r -9 %1.zip %1
