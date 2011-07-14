mkdir -p lib/Linux/x64
mkdir -p lib/Linux/x86
cd lib/Linux/x64
ln -sf ../../../../dist/ReleaseShared64/GNU-Linux-x86/libindigo.so .
ln -sf ../../../../renderer/dist/ReleaseShared64/GNU-Linux-x86/libindigo-renderer.so .
cd ../../..

