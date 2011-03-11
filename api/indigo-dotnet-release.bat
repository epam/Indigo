rmdir /S /Q %1

mkdir %1

copy LICENSE.GPL %1\
copy cs\bin\Release\indigo-cs.dll %1\
copy renderer\cs\bin\Release\indigo-renderer-cs.dll %1\

zip -r -9 %1.zip %1
