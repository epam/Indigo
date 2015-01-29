export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"
ifeq ($(shell uname), Linux)
	INDIGO_LIBRARIES := -lfontconfig -lfreetype
endif

all:indigo
	PKG_CXXFLAGS="-I/usr/local/Cellar/r/3.1.1/R.framework/Resources/include"  R CMD SHLIB -o rindigo.so rindigo.cpp -L$(PWD)/dist/ -L$(PWD)/dist/static/Linux/x64/ -lindigo-static -lz -ltinyxml -lindigo-static $(INDIGO_LIBRARIES)
	./linkhack.py "|gcc|-Ldist/static/Linux/x64/|rindigo.o|-lindigo-static -ltinyxml -lz|./rindigo.so"
indigo:clean
	pwd && ls && cd dist && unzip "indigo-libs-1.2.1-dev5-linux64-static.zip" && cd ..
clean:
	rm -f *.o
	rm -f *.so
