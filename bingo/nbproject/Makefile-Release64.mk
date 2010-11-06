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
FC=
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Release64
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/core/ringo_index.o \
	${OBJECTDIR}/src/oracle/mango_shadow_table.o \
	${OBJECTDIR}/src/core/ringo_aam.o \
	${OBJECTDIR}/src/oracle/mango_fetch_context.o \
	${OBJECTDIR}/src/oracle/ringo_operators.o \
	${OBJECTDIR}/_ext/380065930/chunk_storage.o \
	${OBJECTDIR}/src/oracle/ringo_shadow_fetch.o \
	${OBJECTDIR}/src/oracle/ringo_fetch_context.o \
	${OBJECTDIR}/src/oracle/rowid_saver.o \
	${OBJECTDIR}/src/oracle/mango_oracle.o \
	${OBJECTDIR}/src/core/mango_tautomer.o \
	${OBJECTDIR}/src/core/mango_exact.o \
	${OBJECTDIR}/src/core/mango_similarity.o \
	${OBJECTDIR}/src/oracle/bingo_oracle_config.o \
	${OBJECTDIR}/src/oracle/mango_oracle_util.o \
	${OBJECTDIR}/src/core/mango_substructure.o \
	${OBJECTDIR}/src/core/mango_context.o \
	${OBJECTDIR}/src/core/mango_index.o \
	${OBJECTDIR}/src/oracle/mango_oracle_index_parallel.o \
	${OBJECTDIR}/src/oracle/bingo_profiling.o \
	${OBJECTDIR}/src/core/ringo_substructure.o \
	${OBJECTDIR}/_ext/2025668589/ora_wrap.o \
	${OBJECTDIR}/src/oracle/bingo_storage.o \
	${OBJECTDIR}/src/oracle/ringo_oracle_fetch.o \
	${OBJECTDIR}/src/oracle/mango_oracle_fetch.o \
	${OBJECTDIR}/src/oracle/ringo_shadow_table.o \
	${OBJECTDIR}/src/oracle/bingo_oracle.o \
	${OBJECTDIR}/src/oracle/dll_main.o \
	${OBJECTDIR}/src/oracle/ringo_oracle_util.o \
	${OBJECTDIR}/src/core/ringo_context.o \
	${OBJECTDIR}/src/oracle/mango_fast_index.o \
	${OBJECTDIR}/src/oracle/mango_oracle_index.o \
	${OBJECTDIR}/src/oracle/ringo_oracle_index.o \
	${OBJECTDIR}/src/oracle/rowid_loader.o \
	${OBJECTDIR}/_ext/380065930/shmem_posix.o \
	${OBJECTDIR}/_ext/2025668589/ora_error.o \
	${OBJECTDIR}/src/oracle/bingo_oracle_context.o \
	${OBJECTDIR}/src/oracle/ringo_fast_index.o \
	${OBJECTDIR}/src/oracle/mango_operators.o \
	${OBJECTDIR}/src/oracle/ringo_oracle.o \
	${OBJECTDIR}/src/core/mango_gross.o \
	${OBJECTDIR}/src/oracle/bingo_oracle_parallel.o \
	${OBJECTDIR}/_ext/2025668589/ora_logger.o \
	${OBJECTDIR}/src/oracle/bingo_oracle_util.o \
	${OBJECTDIR}/src/oracle/bingo_fingerprints.o \
	${OBJECTDIR}/src/core/bingo_context.o \
	${OBJECTDIR}/src/oracle/mango_shadow_fetch.o \
	${OBJECTDIR}/_ext/1881957642/nano_posix.o


# C Compiler Flags
CFLAGS=-m64

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../layout/dist/Release64/GNU-Linux-x86/liblayout.a ../reaction/dist/Release64/GNU-Linux-x86/libreaction.a ../molecule/dist/Release64/GNU-Linux-x86/libmolecule.a ../graph/dist/Release64/GNU-Linux-x86/libgraph.a -lz

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release64.mk dist/Release64/GNU-Linux-x86/libbingo.so

dist/Release64/GNU-Linux-x86/libbingo.so: ../layout/dist/Release64/GNU-Linux-x86/liblayout.a

dist/Release64/GNU-Linux-x86/libbingo.so: ../reaction/dist/Release64/GNU-Linux-x86/libreaction.a

dist/Release64/GNU-Linux-x86/libbingo.so: ../molecule/dist/Release64/GNU-Linux-x86/libmolecule.a

dist/Release64/GNU-Linux-x86/libbingo.so: ../graph/dist/Release64/GNU-Linux-x86/libgraph.a

dist/Release64/GNU-Linux-x86/libbingo.so: ${OBJECTFILES}
	${MKDIR} -p dist/Release64/GNU-Linux-x86
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libbingo.so -s -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/core/ringo_index.o: src/core/ringo_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/ringo_index.o src/core/ringo_index.cpp

${OBJECTDIR}/src/oracle/mango_shadow_table.o: src/oracle/mango_shadow_table.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_shadow_table.o src/oracle/mango_shadow_table.cpp

${OBJECTDIR}/src/core/ringo_aam.o: src/core/ringo_aam.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/ringo_aam.o src/core/ringo_aam.cpp

${OBJECTDIR}/src/oracle/mango_fetch_context.o: src/oracle/mango_fetch_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_fetch_context.o src/oracle/mango_fetch_context.cpp

${OBJECTDIR}/src/oracle/ringo_operators.o: src/oracle/ringo_operators.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_operators.o src/oracle/ringo_operators.cpp

${OBJECTDIR}/_ext/380065930/chunk_storage.o: ../common/base_cpp/chunk_storage.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/chunk_storage.o ../common/base_cpp/chunk_storage.cpp

${OBJECTDIR}/src/oracle/ringo_shadow_fetch.o: src/oracle/ringo_shadow_fetch.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_shadow_fetch.o src/oracle/ringo_shadow_fetch.cpp

${OBJECTDIR}/src/oracle/ringo_fetch_context.o: src/oracle/ringo_fetch_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_fetch_context.o src/oracle/ringo_fetch_context.cpp

${OBJECTDIR}/src/oracle/rowid_saver.o: src/oracle/rowid_saver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/rowid_saver.o src/oracle/rowid_saver.cpp

${OBJECTDIR}/src/oracle/mango_oracle.o: src/oracle/mango_oracle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_oracle.o src/oracle/mango_oracle.cpp

${OBJECTDIR}/src/core/mango_tautomer.o: src/core/mango_tautomer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_tautomer.o src/core/mango_tautomer.cpp

${OBJECTDIR}/src/core/mango_exact.o: src/core/mango_exact.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_exact.o src/core/mango_exact.cpp

${OBJECTDIR}/src/core/mango_similarity.o: src/core/mango_similarity.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_similarity.o src/core/mango_similarity.cpp

${OBJECTDIR}/src/oracle/bingo_oracle_config.o: src/oracle/bingo_oracle_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_oracle_config.o src/oracle/bingo_oracle_config.cpp

${OBJECTDIR}/src/oracle/mango_oracle_util.o: src/oracle/mango_oracle_util.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_oracle_util.o src/oracle/mango_oracle_util.cpp

${OBJECTDIR}/src/core/mango_substructure.o: src/core/mango_substructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_substructure.o src/core/mango_substructure.cpp

${OBJECTDIR}/src/core/mango_context.o: src/core/mango_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_context.o src/core/mango_context.cpp

${OBJECTDIR}/src/core/mango_index.o: src/core/mango_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_index.o src/core/mango_index.cpp

${OBJECTDIR}/src/oracle/mango_oracle_index_parallel.o: src/oracle/mango_oracle_index_parallel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_oracle_index_parallel.o src/oracle/mango_oracle_index_parallel.cpp

${OBJECTDIR}/src/oracle/bingo_profiling.o: src/oracle/bingo_profiling.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_profiling.o src/oracle/bingo_profiling.cpp

${OBJECTDIR}/src/core/ringo_substructure.o: src/core/ringo_substructure.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/ringo_substructure.o src/core/ringo_substructure.cpp

${OBJECTDIR}/_ext/2025668589/ora_wrap.o: ../common/oracle/ora_wrap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2025668589
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2025668589/ora_wrap.o ../common/oracle/ora_wrap.cpp

${OBJECTDIR}/src/oracle/bingo_storage.o: src/oracle/bingo_storage.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_storage.o src/oracle/bingo_storage.cpp

${OBJECTDIR}/src/oracle/ringo_oracle_fetch.o: src/oracle/ringo_oracle_fetch.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_oracle_fetch.o src/oracle/ringo_oracle_fetch.cpp

${OBJECTDIR}/src/oracle/mango_oracle_fetch.o: src/oracle/mango_oracle_fetch.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_oracle_fetch.o src/oracle/mango_oracle_fetch.cpp

${OBJECTDIR}/src/oracle/ringo_shadow_table.o: src/oracle/ringo_shadow_table.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_shadow_table.o src/oracle/ringo_shadow_table.cpp

${OBJECTDIR}/src/oracle/bingo_oracle.o: src/oracle/bingo_oracle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_oracle.o src/oracle/bingo_oracle.cpp

${OBJECTDIR}/src/oracle/dll_main.o: src/oracle/dll_main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/dll_main.o src/oracle/dll_main.cpp

${OBJECTDIR}/src/oracle/ringo_oracle_util.o: src/oracle/ringo_oracle_util.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_oracle_util.o src/oracle/ringo_oracle_util.cpp

${OBJECTDIR}/src/core/ringo_context.o: src/core/ringo_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/ringo_context.o src/core/ringo_context.cpp

${OBJECTDIR}/src/oracle/mango_fast_index.o: src/oracle/mango_fast_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_fast_index.o src/oracle/mango_fast_index.cpp

${OBJECTDIR}/src/oracle/mango_oracle_index.o: src/oracle/mango_oracle_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_oracle_index.o src/oracle/mango_oracle_index.cpp

${OBJECTDIR}/src/oracle/ringo_oracle_index.o: src/oracle/ringo_oracle_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_oracle_index.o src/oracle/ringo_oracle_index.cpp

${OBJECTDIR}/src/oracle/rowid_loader.o: src/oracle/rowid_loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/rowid_loader.o src/oracle/rowid_loader.cpp

${OBJECTDIR}/_ext/380065930/shmem_posix.o: ../common/base_cpp/shmem_posix.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/shmem_posix.o ../common/base_cpp/shmem_posix.cpp

${OBJECTDIR}/_ext/2025668589/ora_error.o: ../common/oracle/ora_error.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2025668589
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2025668589/ora_error.o ../common/oracle/ora_error.cpp

${OBJECTDIR}/src/oracle/bingo_oracle_context.o: src/oracle/bingo_oracle_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_oracle_context.o src/oracle/bingo_oracle_context.cpp

${OBJECTDIR}/src/oracle/ringo_fast_index.o: src/oracle/ringo_fast_index.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_fast_index.o src/oracle/ringo_fast_index.cpp

${OBJECTDIR}/src/oracle/mango_operators.o: src/oracle/mango_operators.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_operators.o src/oracle/mango_operators.cpp

${OBJECTDIR}/src/oracle/ringo_oracle.o: src/oracle/ringo_oracle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/ringo_oracle.o src/oracle/ringo_oracle.cpp

${OBJECTDIR}/src/core/mango_gross.o: src/core/mango_gross.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/mango_gross.o src/core/mango_gross.cpp

${OBJECTDIR}/src/oracle/bingo_oracle_parallel.o: src/oracle/bingo_oracle_parallel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_oracle_parallel.o src/oracle/bingo_oracle_parallel.cpp

${OBJECTDIR}/_ext/2025668589/ora_logger.o: ../common/oracle/ora_logger.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2025668589
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2025668589/ora_logger.o ../common/oracle/ora_logger.cpp

${OBJECTDIR}/src/oracle/bingo_oracle_util.o: src/oracle/bingo_oracle_util.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_oracle_util.o src/oracle/bingo_oracle_util.cpp

${OBJECTDIR}/src/oracle/bingo_fingerprints.o: src/oracle/bingo_fingerprints.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/bingo_fingerprints.o src/oracle/bingo_fingerprints.cpp

${OBJECTDIR}/src/core/bingo_context.o: src/core/bingo_context.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/core
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/core/bingo_context.o src/core/bingo_context.cpp

${OBJECTDIR}/src/oracle/mango_shadow_fetch.o: src/oracle/mango_shadow_fetch.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/oracle
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -Isrc -I../oci/include -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/oracle/mango_shadow_fetch.o src/oracle/mango_shadow_fetch.cpp

${OBJECTDIR}/_ext/1881957642/nano_posix.o: ../common/base_c/nano_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I.. -I../common -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/nano_posix.o ../common/base_c/nano_posix.c

# Subprojects
.build-subprojects:
	cd ../layout && ${MAKE}  -f Makefile CONF=Release64
	cd ../reaction && ${MAKE}  -f Makefile CONF=Release64
	cd ../molecule && ${MAKE}  -f Makefile CONF=Release64
	cd ../graph && ${MAKE}  -f Makefile CONF=Release64

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release64
	${RM} dist/Release64/GNU-Linux-x86/libbingo.so

# Subprojects
.clean-subprojects:
	cd ../layout && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../reaction && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../molecule && ${MAKE}  -f Makefile CONF=Release64 clean
	cd ../graph && ${MAKE}  -f Makefile CONF=Release64 clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
