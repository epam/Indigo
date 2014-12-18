export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"
ifeq ($(shell uname), Linux)
	INDIGO_LIBRARIES := -lfontconfig -lfreetype
endif

all:indigo
	PKG_CXXFLAGS="-I/usr/local/Cellar/r/3.1.1/R.framework/Resources/include -I$(PWD)/api -I$(PWD)/api/plugins/renderer -I$(PWD)/api/plugins/inchi"  R CMD SHLIB -o rindigo.so rindigo.cpp -I$(PWD)/api -L$(PWD)/dist/static/Linux/x64/ -L$(PWD)/build/dist/Mac/10.6/lib -L$(PWD)/build/dist/Mac/10.7/lib -L$(PWD)/build/dist/Mac/10.8/lib -L$(PWD)/build/dist/Mac/10.9/lib -L$(PWD)/build/dist/Mac/10.10/lib -L$(PWD)/build/dist/Linux/x64/lib -L$(PWD)/build/dist/Linux/x86/lib -L$(PWD)/build/dist/Win/x64/lib -L$(PWD)/build/dist/Win/x86/lib -L$(PWD)/dist/static/Linux/x64/ -lindigo-static -lz -ltinyxml -lindigo-static $(INDIGO_LIBRARIES)
        #$(shell ./common/cmake/linkhack.py "gcc|-Ldist/static/Linux/x64/|rindigo.o|-lindigo-static -ltinyxml -lz|./rindigo.so")
	./common/cmake/linkhack.py "|gcc|-Ldist/static/Linux/x64/|rindigo.o|-lindigo-static -ltinyxml -lz|./rindigo.so"
	# PKG_CXXFLAGS="-I/usr/local/Cellar/r/3.1.1/R.framework/Resources/include -I$(PWD)/api -I$(PWD)/api/plugins/renderer -I$(PWD)/api/plugins/inchi"  R CMD SHLIB -o test.so test.cpp -I$(PWD)/api -I$(PWD)/api/plugins/renderer -I$(PWD)/api/plugins/inchi -L$(PWD)/dist/static/Linux/x64/ -L$(PWD)/build/dist/Mac/10.6/lib -L$(PWD)/build/dist/Mac/10.7/lib -L$(PWD)/build/dist/Mac/10.8/lib -L$(PWD)/build/dist/Mac/10.9/lib -L$(PWD)/build/dist/Mac/10.10/lib -L$(PWD)/build/dist/Linux/x64/lib -L$(PWD)/build/dist/Linux/x86/lib -L$(PWD)/build/dist/Win/x64/lib -L$(PWD)/build/dist/Win/x86/lib -L$(PWD)/dist/shared/Linux/x64/ -lindigo-static -lz -ltinyxml -lindigo-inchi-static -lindigo-renderer-static -lpng -lpixman -lcairo -linchi -lindigo-static -lz -ltinyxml -lindigo-inchi-static -lindigo-renderer-static -lpng -lpixman -lcairo -linchi -lindigo-static -lz -ltinyxml -lindigo-inchi-static -lindigo-renderer-static -lpng -lpixman -lcairo -linchi $(INDIGO_LIBRARIES)
indigo:clean
	$(shell /usr/bin/python build_scripts/indigo-release-libs.py --with-static --preset=linux64-universal >/dev/null 2>/dev/null)
	pwd && ls && cd dist && unzip "indigo-libs-1.2.1-dev5-linux64-static.zip" && cd ..

clean:
	rm -f *.o
	rm -f *.so
	rm -rf dist
