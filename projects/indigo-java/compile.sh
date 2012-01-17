if [ -z $JAVA_HOME ]; then
  echo "'JAVA_HOME' is not defined. Will use default commands: 'jar' and 'javac'";
  export EXEC_JAR=jar;
  export EXEC_JAVAC=javac;
else
  export EXEC_JAR=$JAVA_HOME/bin/jar;
  export EXEC_JAVAC=$JAVA_HOME/bin/javac;
fi
indigoJavaPath=../../api/java/src/com/ggasoftware/indigo
mkdir dist
rm -rf $indigoJavaPath/*.class
$EXEC_JAVAC -cp ../../common/jna/jna.jar $indigoJavaPath/*.java
$EXEC_JAR cvf dist/indigo.jar $indigoJavaPath/*.class
rm -rf $indigoJavaPath/*.class
