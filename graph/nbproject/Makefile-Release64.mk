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
	${OBJECTDIR}/src/graph_perfect_matching.o \
	${OBJECTDIR}/src/subgraph_hash.o \
	${OBJECTDIR}/src/embedding_enumerator.o \
	${OBJECTDIR}/src/skew_symmetric_flow_finder.o \
	${OBJECTDIR}/src/cycle_enumerator.o \
	${OBJECTDIR}/_ext/1705077469/vec2f.o \
	${OBJECTDIR}/_ext/380065930/os_thread_wrapper.o \
	${OBJECTDIR}/src/morgan_code.o \
	${OBJECTDIR}/src/path_enumerator.o \
	${OBJECTDIR}/src/automorphism_search.o \
	${OBJECTDIR}/src/graph_highlighting.o \
	${OBJECTDIR}/_ext/1705077469/line3f.o \
	${OBJECTDIR}/_ext/1881957642/os_tls_posix.o \
	${OBJECTDIR}/src/filter.o \
	${OBJECTDIR}/_ext/380065930/smart_output.o \
	${OBJECTDIR}/_ext/380065930/string_pool.o \
	${OBJECTDIR}/src/cycle_basis.o \
	${OBJECTDIR}/_ext/380065930/io_base.o \
	${OBJECTDIR}/src/simple_cycle_basis.o \
	${OBJECTDIR}/src/spanning_tree.o \
	${OBJECTDIR}/_ext/380065930/d_bitset.o \
	${OBJECTDIR}/_ext/380065930/exception.o \
	${OBJECTDIR}/_ext/380065930/output.o \
	${OBJECTDIR}/src/graph_subtree_enumerator.o \
	${OBJECTDIR}/_ext/1705077469/matr3x3d.o \
	${OBJECTDIR}/_ext/380065930/tlscont.o \
	${OBJECTDIR}/_ext/1705077469/lseg3f.o \
	${OBJECTDIR}/src/biconnected_decomposer.o \
	${OBJECTDIR}/_ext/1705077469/vec3f.o \
	${OBJECTDIR}/src/graph.o \
	${OBJECTDIR}/src/graph_subchain_enumerator.o \
	${OBJECTDIR}/src/skew_symmetric_network.o \
	${OBJECTDIR}/_ext/1881957642/os_thread_posix.o \
	${OBJECTDIR}/_ext/380065930/profiling.o \
	${OBJECTDIR}/_ext/380065930/os_sync_wrapper.o \
	${OBJECTDIR}/_ext/1881957642/os_sync_posix.o \
	${OBJECTDIR}/src/aux_path_finder.o \
	${OBJECTDIR}/_ext/1705077469/transform3f.o \
	${OBJECTDIR}/src/embeddings_storage.o \
	${OBJECTDIR}/src/edge_rotation_matcher.o \
	${OBJECTDIR}/src/graph_affine_matcher.o \
	${OBJECTDIR}/src/shortest_path_finder.o \
	${OBJECTDIR}/src/scaffold_detection.o \
	${OBJECTDIR}/_ext/1705077469/plane3f.o \
	${OBJECTDIR}/_ext/1881957642/nano_posix.o \
	${OBJECTDIR}/src/dfs_walk.o \
	${OBJECTDIR}/src/graph_constrained_bmatching_finder.o \
	${OBJECTDIR}/_ext/1705077469/best_fit.o \
	${OBJECTDIR}/src/graph_decomposer.o \
	${OBJECTDIR}/src/edge_subgraph_enumerator.o \
	${OBJECTDIR}/src/max_common_subgraph.o \
	${OBJECTDIR}/_ext/380065930/crc32.o


# C Compiler Flags
CFLAGS=-m64 -fPIC

# CC Compiler Flags
CCFLAGS=-m64 -fPIC
CXXFLAGS=-m64 -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release64.mk dist/Release64/GNU-Linux-x86/libgraph.a

dist/Release64/GNU-Linux-x86/libgraph.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release64/GNU-Linux-x86
	${RM} dist/Release64/GNU-Linux-x86/libgraph.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libgraph.a ${OBJECTFILES} 
	$(RANLIB) dist/Release64/GNU-Linux-x86/libgraph.a

${OBJECTDIR}/src/graph_perfect_matching.o: src/graph_perfect_matching.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_perfect_matching.o src/graph_perfect_matching.cpp

${OBJECTDIR}/src/subgraph_hash.o: src/subgraph_hash.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/subgraph_hash.o src/subgraph_hash.cpp

${OBJECTDIR}/src/embedding_enumerator.o: src/embedding_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/embedding_enumerator.o src/embedding_enumerator.cpp

${OBJECTDIR}/src/skew_symmetric_flow_finder.o: src/skew_symmetric_flow_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/skew_symmetric_flow_finder.o src/skew_symmetric_flow_finder.cpp

${OBJECTDIR}/src/cycle_enumerator.o: src/cycle_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cycle_enumerator.o src/cycle_enumerator.cpp

${OBJECTDIR}/_ext/1705077469/vec2f.o: ../common/math/vec2f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/vec2f.o ../common/math/vec2f.cpp

${OBJECTDIR}/_ext/380065930/os_thread_wrapper.o: ../common/base_cpp/os_thread_wrapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/os_thread_wrapper.o ../common/base_cpp/os_thread_wrapper.cpp

${OBJECTDIR}/src/morgan_code.o: src/morgan_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/morgan_code.o src/morgan_code.cpp

${OBJECTDIR}/src/path_enumerator.o: src/path_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/path_enumerator.o src/path_enumerator.cpp

${OBJECTDIR}/src/automorphism_search.o: src/automorphism_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/automorphism_search.o src/automorphism_search.cpp

${OBJECTDIR}/src/graph_highlighting.o: src/graph_highlighting.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_highlighting.o src/graph_highlighting.cpp

${OBJECTDIR}/_ext/1705077469/line3f.o: ../common/math/line3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/line3f.o ../common/math/line3f.cpp

${OBJECTDIR}/_ext/1881957642/os_tls_posix.o: ../common/base_c/os_tls_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/os_tls_posix.o ../common/base_c/os_tls_posix.c

${OBJECTDIR}/src/filter.o: src/filter.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/filter.o src/filter.cpp

${OBJECTDIR}/_ext/380065930/smart_output.o: ../common/base_cpp/smart_output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/smart_output.o ../common/base_cpp/smart_output.cpp

${OBJECTDIR}/_ext/380065930/string_pool.o: ../common/base_cpp/string_pool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/string_pool.o ../common/base_cpp/string_pool.cpp

${OBJECTDIR}/src/cycle_basis.o: src/cycle_basis.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cycle_basis.o src/cycle_basis.cpp

${OBJECTDIR}/_ext/380065930/io_base.o: ../common/base_cpp/io_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/io_base.o ../common/base_cpp/io_base.cpp

${OBJECTDIR}/src/simple_cycle_basis.o: src/simple_cycle_basis.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/simple_cycle_basis.o src/simple_cycle_basis.cpp

${OBJECTDIR}/src/spanning_tree.o: src/spanning_tree.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/spanning_tree.o src/spanning_tree.cpp

${OBJECTDIR}/_ext/380065930/d_bitset.o: ../common/base_cpp/d_bitset.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/d_bitset.o ../common/base_cpp/d_bitset.cpp

${OBJECTDIR}/_ext/380065930/exception.o: ../common/base_cpp/exception.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/exception.o ../common/base_cpp/exception.cpp

${OBJECTDIR}/_ext/380065930/output.o: ../common/base_cpp/output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/output.o ../common/base_cpp/output.cpp

${OBJECTDIR}/src/graph_subtree_enumerator.o: src/graph_subtree_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_subtree_enumerator.o src/graph_subtree_enumerator.cpp

${OBJECTDIR}/_ext/1705077469/matr3x3d.o: ../common/math/matr3x3d.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/matr3x3d.o ../common/math/matr3x3d.cpp

${OBJECTDIR}/_ext/380065930/tlscont.o: ../common/base_cpp/tlscont.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/tlscont.o ../common/base_cpp/tlscont.cpp

${OBJECTDIR}/_ext/1705077469/lseg3f.o: ../common/math/lseg3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/lseg3f.o ../common/math/lseg3f.cpp

${OBJECTDIR}/src/biconnected_decomposer.o: src/biconnected_decomposer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/biconnected_decomposer.o src/biconnected_decomposer.cpp

${OBJECTDIR}/_ext/1705077469/vec3f.o: ../common/math/vec3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/vec3f.o ../common/math/vec3f.cpp

${OBJECTDIR}/src/graph.o: src/graph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph.o src/graph.cpp

${OBJECTDIR}/src/graph_subchain_enumerator.o: src/graph_subchain_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_subchain_enumerator.o src/graph_subchain_enumerator.cpp

${OBJECTDIR}/src/skew_symmetric_network.o: src/skew_symmetric_network.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/skew_symmetric_network.o src/skew_symmetric_network.cpp

${OBJECTDIR}/_ext/1881957642/os_thread_posix.o: ../common/base_c/os_thread_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/os_thread_posix.o ../common/base_c/os_thread_posix.c

${OBJECTDIR}/_ext/380065930/profiling.o: ../common/base_cpp/profiling.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/profiling.o ../common/base_cpp/profiling.cpp

${OBJECTDIR}/_ext/380065930/os_sync_wrapper.o: ../common/base_cpp/os_sync_wrapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/os_sync_wrapper.o ../common/base_cpp/os_sync_wrapper.cpp

${OBJECTDIR}/_ext/1881957642/os_sync_posix.o: ../common/base_c/os_sync_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/os_sync_posix.o ../common/base_c/os_sync_posix.c

${OBJECTDIR}/src/aux_path_finder.o: src/aux_path_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/aux_path_finder.o src/aux_path_finder.cpp

${OBJECTDIR}/_ext/1705077469/transform3f.o: ../common/math/transform3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/transform3f.o ../common/math/transform3f.cpp

${OBJECTDIR}/src/embeddings_storage.o: src/embeddings_storage.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/embeddings_storage.o src/embeddings_storage.cpp

${OBJECTDIR}/src/edge_rotation_matcher.o: src/edge_rotation_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/edge_rotation_matcher.o src/edge_rotation_matcher.cpp

${OBJECTDIR}/src/graph_affine_matcher.o: src/graph_affine_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_affine_matcher.o src/graph_affine_matcher.cpp

${OBJECTDIR}/src/shortest_path_finder.o: src/shortest_path_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/shortest_path_finder.o src/shortest_path_finder.cpp

${OBJECTDIR}/src/scaffold_detection.o: src/scaffold_detection.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/scaffold_detection.o src/scaffold_detection.cpp

${OBJECTDIR}/_ext/1705077469/plane3f.o: ../common/math/plane3f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/plane3f.o ../common/math/plane3f.cpp

${OBJECTDIR}/_ext/1881957642/nano_posix.o: ../common/base_c/nano_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881957642
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1881957642/nano_posix.o ../common/base_c/nano_posix.c

${OBJECTDIR}/src/dfs_walk.o: src/dfs_walk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/dfs_walk.o src/dfs_walk.cpp

${OBJECTDIR}/src/graph_constrained_bmatching_finder.o: src/graph_constrained_bmatching_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_constrained_bmatching_finder.o src/graph_constrained_bmatching_finder.cpp

${OBJECTDIR}/_ext/1705077469/best_fit.o: ../common/math/best_fit.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1705077469
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1705077469/best_fit.o ../common/math/best_fit.cpp

${OBJECTDIR}/src/graph_decomposer.o: src/graph_decomposer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_decomposer.o src/graph_decomposer.cpp

${OBJECTDIR}/src/edge_subgraph_enumerator.o: src/edge_subgraph_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/edge_subgraph_enumerator.o src/edge_subgraph_enumerator.cpp

${OBJECTDIR}/src/max_common_subgraph.o: src/max_common_subgraph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/max_common_subgraph.o src/max_common_subgraph.cpp

${OBJECTDIR}/_ext/380065930/crc32.o: ../common/base_cpp/crc32.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/380065930
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/380065930/crc32.o ../common/base_cpp/crc32.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release64
	${RM} dist/Release64/GNU-Linux-x86/libgraph.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
