if [ -z $JAVA_HOME ]; then
  echo "'JAVA_HOME' is not defined. Will use default commands: 'jar' and 'javac'";
  export EXEC_JAR=jar;
  export EXEC_JAVAC=javac;
else
  export EXEC_JAR=$JAVA_HOME/bin/jar;
  export EXEC_JAVAC=$JAVA_HOME/bin/javac;
fi
indigoJavaSourcePath=com/ggasoftware/indigo
indigoJavaPath=../../api/java/src/
distPath=$PWD
rm -rf dist
mkdir dist
cd $indigoJavaPath
rm -rf $indigoJavaSourcePath/*.class
$EXEC_JAVAC -cp ../../../common/jna/jna.jar $indigoJavaSourcePath/*.java
$EXEC_JAR cvf $distPath/dist/indigo.jar $indigoJavaSourcePath/*.class
rm -rf $indigoJavaSourcePath/*.class
cd $distPath

