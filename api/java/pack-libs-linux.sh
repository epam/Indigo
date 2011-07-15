if [ -z $JAVA_HOME ]; then
  echo "'JAVA_HOME' is not defined. Will use default commands: 'jar' and 'javac'";
  export EXEC_JAR=jar;
  export EXEC_JAVAC=javac;
else
  export EXEC_JAR=$JAVA_HOME/bin/jar;
  export EXEC_JAVAC=$JAVA_HOME/bin/javac;
fi

mkdir -p com/ggasoftware/indigo/Linux/x86
mkdir -p com/ggasoftware/indigo/Linux/x64

cp ../dist/ReleaseShared32/GNU-Linux-x86/libindigo.so com/ggasoftware/indigo/Linux/x86
cp ../dist/ReleaseShared64/GNU-Linux-x86/libindigo.so com/ggasoftware/indigo/Linux/x64

$EXEC_JAR uf dist/indigo.jar com/ggasoftware/indigo/Linux/x86/libindigo.so 
$EXEC_JAR uf dist/indigo.jar com/ggasoftware/indigo/Linux/x64/libindigo.so 

rm com/ggasoftware/indigo/Linux/x86/libindigo.so
rm com/ggasoftware/indigo/Linux/x64/libindigo.so
