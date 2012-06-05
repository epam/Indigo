#!/bin/sh
mkdir -p lib/Linux/x86
mkdir -p lib/Linux/x64
cd lib/Linux/x64
ln -sf ../../../../api/dist/DebugShared/GNU-Linux-x86/libindigo.so .
ln -sf ../../../../api/renderer/dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so .
cd ../x86
ln -sf ../../../../api/dist/DebugShared/GNU-Linux-x86/libindigo.so .
ln -sf ../../../../api/renderer/dist/DebugShared/GNU-Linux-x86/libindigo-renderer.so .
cd ../../..
