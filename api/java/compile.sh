if [ -z $JAVA_HOME ]; then
  echo "'JAVA_HOME' is not defined. Will use default commands: 'jar' and 'javac'";
  export EXEC_JAR=jar;
  export EXEC_JAVAC=javac;
else
  export EXEC_JAR=$JAVA_HOME/bin/jar;
  export EXEC_JAVAC=$JAVA_HOME/bin/javac;
fi
mkdir dist
cd src
rm -rf com/ggasoftware/indigo/*.class
$EXEC_JAVAC -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
$EXEC_JAR cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class
cd ..

