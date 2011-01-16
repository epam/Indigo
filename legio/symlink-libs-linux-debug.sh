mkdir -p lib/Linux/x86
mkdir -p lib/Linux/x64
cd lib/Linux/x64
ln -sf ../../../../api/jni/dist/Debug/GNU-Linux-x86/libindigo-jni.so .
ln -sf ../../../../api/renderer/jni/dist/Debug/GNU-Linux-x86/libindigo-renderer-jni.so .
cd ../x86
ln -sf ../../../../api/renderer/jni/dist/Debug/GNU-Linux-x86/libindigo-renderer-jni.so .

