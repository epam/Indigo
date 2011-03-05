#!/bin/sh
mkdir -p com/ggasoftware/indigo/Mac/10.5
mkdir -p com/ggasoftware/indigo/Mac/10.6
cp ../build/Release10.5/libindigo.dylib com/ggasoftware/indigo/Mac/10.5/
cp ../build/Release10.6/libindigo.dylib com/ggasoftware/indigo/Mac/10.6/
jar uf dist/indigo.jar com/ggasoftware/indigo/Mac/10.5/libindigo.dylib
jar uf dist/indigo.jar com/ggasoftware/indigo/Mac/10.6/libindigo.dylib
rm -rf com

