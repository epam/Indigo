mkdir -p lib/Linux/x64
mkdir -p lib/Linux/x86
cd lib/Linux/x32
ln -sf ../../../../dist/ReleaseShared32/GNU-Linux-x86/libindigo.so .
ln -sf ../../../../renderer/dist/ReleaseShared32/GNU-Linux-x86/libindigo-renderer.so .
cd ../x86
ln -sf ../../../../renderer/dist/ReleaseShared32/GNU-Linux-x86/libindigo-renderer.so .
cd ../../..

