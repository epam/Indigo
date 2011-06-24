for osxver in '10.5' '10.6'; do
   mkdir -p lib/Mac/$osxver
   cd lib/Mac/$osxver
   ln -sf ../../../../build/Release$osxver/libindigo.dylib .
   ln -sf ../../../../renderer/build/Release$osxver/libindigo-renderer.dylib .
   cd ../../..
done
