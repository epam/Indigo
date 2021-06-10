repo_root := $(shell pwd)/../../..
revision = $(shell git rev-parse --short HEAD)

all:indigo_build
	PKG_CXXFLAGS="-I$(repo_root)/api/cpp/indigo -I$(repo_root)/api/cpp/indigo-renderer" R CMD SHLIB -L/tmp/indigo-build-$(revision)/lib -o rindigo.so rindigo.cpp -lindigo-renderer-static -lindigo-static -lindigo-core -lrender2d -linchi
indigo_build:clean
	mkdir -p /tmp/indigo-build-$(revision) && \
	cd /tmp/indigo-build-$(revision) && \
	cmake $(repo_root) && \
	make -j indigo-static indigo-renderer-static
clean:
	rm -f *.o
	rm -f *.so
	rm -f *.dll
