#!/bin/sh
mkdir -p com/ggasoftware/indigo/Mac/10.5/
mkdir -p com/ggasoftware/indigo/Mac/10.6/
cp ../build/Release10.5/libindigo-renderer.dylib com/ggasoftware/indigo/Mac/10.5
cp ../build/Release10.6/libindigo-renderer.dylib com/ggasoftware/indigo/Mac/10.6
jar uf dist/indigo-renderer.jar com/ggasoftware/indigo/Mac/10.5/libindigo-renderer.dylib 
jar uf dist/indigo-renderer.jar com/ggasoftware/indigo/Mac/10.6/libindigo-renderer.dylib
#rm -r com
