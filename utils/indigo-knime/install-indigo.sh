#!/bin/sh

INDIGO_VERSION=1.3.0beta.r16
INDIGO_LIB_DIR=./com.epam.indigo.knime.plugin/lib

if [ ! -d $INDIGO_LIB_DIR/ ] 
  then    
    mkdir -p $INDIGO_LIB_DIR/
fi

# Indigo tools
for item in "indigo" "indigo-inchi" "indigo-renderer"
do
    mvn dependency:get \
        -Dartifact=com.epam.indigo:$item:$INDIGO_VERSION:jar \
        -Ddest=$INDIGO_LIB_DIR/$item.jar 
done

# JNA
mvn dependency:get \
    -Dartifact=net.java.dev.jna:jna:3.5.0:jar \
    -Ddest=$INDIGO_LIB_DIR/jna.jar 
