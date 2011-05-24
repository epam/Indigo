#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/pg_common/bingo_pg_buffer_cache.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_import.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_ext_bitset.o \
	${OBJECTDIR}/_ext/1019403175/mango_gross.o \
	${OBJECTDIR}/_ext/1019403175/ringo_exact.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_options.o \
	${OBJECTDIR}/_ext/393906193/mango_core_c_parallel.o \
	${OBJECTDIR}/_ext/1019403175/ringo_context.o \
	${OBJECTDIR}/_ext/1019403175/mango_exact.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_search.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_config.o \
	${OBJECTDIR}/_ext/1019403175/mango_context.o \
	${OBJECTDIR}/_ext/393906193/ringo_core_c_parallel.o \
	${OBJECTDIR}/_ext/1149244103/chunk_storage.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_text.o \
	${OBJECTDIR}/src/pg_am/pg_mango_match.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_cursor.o \
	${OBJECTDIR}/_ext/393906193/ringo_core_c.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_build_engine.o \
	${OBJECTDIR}/_ext/393906193/mango_core_c.o \
	${OBJECTDIR}/_ext/1019403175/mango_tautomer.o \
	${OBJECTDIR}/src/pg_am/pg_ringo_match.o \
	${OBJECTDIR}/_ext/1019403175/ringo_index.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_search.o \
	${OBJECTDIR}/_ext/1019403175/mango_substructure.o \
	${OBJECTDIR}/_ext/1019403175/ringo_substructure.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_buffer.o \
	${OBJECTDIR}/src/pg_core/mango_pg_search_engine.o \
	${OBJECTDIR}/_ext/1019403175/ringo_aam.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_build.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_common.o \
	${OBJECTDIR}/_ext/1019403175/bingo_context.o \
	${OBJECTDIR}/_ext/1019403175/mango_similarity.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_search_engine.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_build.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_costestimate.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_gist.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_utils.o \
	${OBJECTDIR}/src/pg_core/bingo_pg_index.o \
	${OBJECTDIR}/_ext/393906193/bingo_core_c_parallel.o \
	${OBJECTDIR}/_ext/1019403175/mango_index.o \
	${OBJECTDIR}/_ext/393906193/bingo_core_c.o \
	${OBJECTDIR}/src/pg_common/bingo_pg_section.o \
	${OBJECTDIR}/src/pg_core/mango_pg_build_engine.o \
	${OBJECTDIR}/src/pg_am/pg_bingo_update.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../layout/dist/Debug/GNU-Linux-x86/liblayout.a ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a -lz -lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk bin/bingo_postgres.so

bin/bingo_postgres.so: ../../layout/dist/Debug/GNU-Linux-x86/liblayout.a

bin/bingo_postgres.so: ../../reaction/dist/Debug/GNU-Linux-x86/libreaction.a

bin/bingo_postgres.so: ../../molecule/dist/Debug/GNU-Linux-x86/libmolecule.a

bin/bingo_postgres.so: ../../graph/dist/Debug/GNU-Linux-x86/libgraph.a

bin/bingo_postgres.so: ../../tinyxml/dist/Debug/GNU-Linux-x86/libtinyxml.a

bin/bingo_postgres.so: ${OBJECTFILES}
	${MKDIR} -p bin
	${LINK.cc} -shared -o bin/bingo_postgres.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/pg_common/bingo_pg_buffer_cache.o: src/pg_common/bingo_pg_buffer_cache.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_buffer_cache.o src/pg_common/bingo_pg_buffer_cache.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_import.o: src/pg_am/pg_bingo_import.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_import.o src/pg_am/pg_bingo_import.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_ext_bitset.o: src/pg_common/bingo_pg_ext_bitset.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_ext_bitset.o src/pg_common/bingo_pg_ext_bitset.cpp

${OBJECTDIR}/_ext/1019403175/mango_gross.o: ../src/core/mango_gross.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_gross.o ../src/core/mango_gross.cpp

${OBJECTDIR}/_ext/1019403175/ringo_exact.o: ../src/core/ringo_exact.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/ringo_exact.o ../src/core/ringo_exact.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_options.o: src/pg_am/pg_bingo_options.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_options.o src/pg_am/pg_bingo_options.cpp

${OBJECTDIR}/_ext/393906193/mango_core_c_parallel.o: ../src/core-c/mango_core_c_parallel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/mango_core_c_parallel.o ../src/core-c/mango_core_c_parallel.cpp

${OBJECTDIR}/_ext/1019403175/ringo_context.o: ../src/core/ringo_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/ringo_context.o ../src/core/ringo_context.cpp

${OBJECTDIR}/_ext/1019403175/mango_exact.o: ../src/core/mango_exact.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_exact.o ../src/core/mango_exact.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_search.o: src/pg_am/pg_bingo_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_search.o src/pg_am/pg_bingo_search.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_config.o: src/pg_core/bingo_pg_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_config.o src/pg_core/bingo_pg_config.cpp

${OBJECTDIR}/_ext/1019403175/mango_context.o: ../src/core/mango_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_context.o ../src/core/mango_context.cpp

${OBJECTDIR}/_ext/393906193/ringo_core_c_parallel.o: ../src/core-c/ringo_core_c_parallel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/ringo_core_c_parallel.o ../src/core-c/ringo_core_c_parallel.cpp

${OBJECTDIR}/_ext/1149244103/chunk_storage.o: ../../common/base_cpp/chunk_storage.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1149244103
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1149244103/chunk_storage.o ../../common/base_cpp/chunk_storage.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_text.o: src/pg_common/bingo_pg_text.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_text.o src/pg_common/bingo_pg_text.cpp

${OBJECTDIR}/src/pg_am/pg_mango_match.o: src/pg_am/pg_mango_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_mango_match.o src/pg_am/pg_mango_match.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_cursor.o: src/pg_common/bingo_pg_cursor.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_cursor.o src/pg_common/bingo_pg_cursor.cpp

${OBJECTDIR}/_ext/393906193/ringo_core_c.o: ../src/core-c/ringo_core_c.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/ringo_core_c.o ../src/core-c/ringo_core_c.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_build_engine.o: src/pg_core/bingo_pg_build_engine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_build_engine.o src/pg_core/bingo_pg_build_engine.cpp

${OBJECTDIR}/_ext/393906193/mango_core_c.o: ../src/core-c/mango_core_c.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/mango_core_c.o ../src/core-c/mango_core_c.cpp

${OBJECTDIR}/_ext/1019403175/mango_tautomer.o: ../src/core/mango_tautomer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_tautomer.o ../src/core/mango_tautomer.cpp

${OBJECTDIR}/src/pg_am/pg_ringo_match.o: src/pg_am/pg_ringo_match.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_ringo_match.o src/pg_am/pg_ringo_match.cpp

${OBJECTDIR}/_ext/1019403175/ringo_index.o: ../src/core/ringo_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/ringo_index.o ../src/core/ringo_index.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_search.o: src/pg_core/bingo_pg_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_search.o src/pg_core/bingo_pg_search.cpp

${OBJECTDIR}/_ext/1019403175/mango_substructure.o: ../src/core/mango_substructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_substructure.o ../src/core/mango_substructure.cpp

${OBJECTDIR}/_ext/1019403175/ringo_substructure.o: ../src/core/ringo_substructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/ringo_substructure.o ../src/core/ringo_substructure.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_buffer.o: src/pg_common/bingo_pg_buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_buffer.o src/pg_common/bingo_pg_buffer.cpp

${OBJECTDIR}/src/pg_core/mango_pg_search_engine.o: src/pg_core/mango_pg_search_engine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/mango_pg_search_engine.o src/pg_core/mango_pg_search_engine.cpp

${OBJECTDIR}/_ext/1019403175/ringo_aam.o: ../src/core/ringo_aam.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/ringo_aam.o ../src/core/ringo_aam.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_build.o: src/pg_core/bingo_pg_build.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_build.o src/pg_core/bingo_pg_build.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_common.o: src/pg_common/bingo_pg_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_common.o src/pg_common/bingo_pg_common.cpp

${OBJECTDIR}/_ext/1019403175/bingo_context.o: ../src/core/bingo_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/bingo_context.o ../src/core/bingo_context.cpp

${OBJECTDIR}/_ext/1019403175/mango_similarity.o: ../src/core/mango_similarity.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_similarity.o ../src/core/mango_similarity.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_search_engine.o: src/pg_core/bingo_pg_search_engine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_search_engine.o src/pg_core/bingo_pg_search_engine.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_build.o: src/pg_am/pg_bingo_build.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_build.o src/pg_am/pg_bingo_build.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_costestimate.o: src/pg_am/pg_bingo_costestimate.c 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.c) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_costestimate.o src/pg_am/pg_bingo_costestimate.c

${OBJECTDIR}/src/pg_am/pg_bingo_gist.o: src/pg_am/pg_bingo_gist.c 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.c) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_gist.o src/pg_am/pg_bingo_gist.c

${OBJECTDIR}/src/pg_am/pg_bingo_utils.o: src/pg_am/pg_bingo_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_utils.o src/pg_am/pg_bingo_utils.cpp

${OBJECTDIR}/src/pg_core/bingo_pg_index.o: src/pg_core/bingo_pg_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/bingo_pg_index.o src/pg_core/bingo_pg_index.cpp

${OBJECTDIR}/_ext/393906193/bingo_core_c_parallel.o: ../src/core-c/bingo_core_c_parallel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/bingo_core_c_parallel.o ../src/core-c/bingo_core_c_parallel.cpp

${OBJECTDIR}/_ext/1019403175/mango_index.o: ../src/core/mango_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1019403175
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1019403175/mango_index.o ../src/core/mango_index.cpp

${OBJECTDIR}/_ext/393906193/bingo_core_c.o: ../src/core-c/bingo_core_c.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/393906193
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/393906193/bingo_core_c.o ../src/core-c/bingo_core_c.cpp

${OBJECTDIR}/src/pg_common/bingo_pg_section.o: src/pg_common/bingo_pg_section.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_common
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_common/bingo_pg_section.o src/pg_common/bingo_pg_section.cpp

${OBJECTDIR}/src/pg_core/mango_pg_build_engine.o: src/pg_core/mango_pg_build_engine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_core
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_core/mango_pg_build_engine.o src/pg_core/mango_pg_build_engine.cpp

${OBJECTDIR}/src/pg_am/pg_bingo_update.o: src/pg_am/pg_bingo_update.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/pg_am
	${RM} $@.d
	$(COMPILE.cc) -g -I../.. -I../../common -Isrc/pg_common -Isrc/pg_core -I../src/core-c -I../src -Iinclude -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/pg_am/pg_bingo_update.o src/pg_am/pg_bingo_update.cpp

# Subprojects
.build-subprojects:
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} bin/bingo_postgres.so

# Subprojects
.clean-subprojects:
	cd ../../layout && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../reaction && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../molecule && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../graph && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../tinyxml && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
