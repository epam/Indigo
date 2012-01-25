if [ -z $JAVA_HOME ]; then
  echo "'JAVA_HOME' is not defined. Will use default commands: 'jar' and 'javac'";
  export EXEC_JAR=jar;
  export EXEC_JAVAC=javac;
else
  export EXEC_JAR=$JAVA_HOME/bin/jar;
  export EXEC_JAVAC=$JAVA_HOME/bin/javac;
fi
indigoJavaSourcePath=../../api/java/src/com/ggasoftware/indigo
indigoRendererJavaPath=../../api/renderer/java/src/
distPath=$PWD/dist
rm -rf $distPath
mkdir $distPath
cd ../../api/renderer/java/src/
rm -rf com/ggasoftware/indigo/*.class
$EXEC_JAVAC -cp ../../../../projects/indigo-java/dist/indigo.jar:../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
$EXEC_JAR cvf $distPath/indigo-renderer.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class