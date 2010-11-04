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
CCADMIN=CCadmin
RANLIB=ranlib
CC=icc
CCC=icc
CXX=icc
FC=
AS=

# Macros
CND_PLATFORM=ICC-Linux-x86
CND_CONF=ReleaseIntel64
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/cycle_enumerator.o \
	${OBJECTDIR}/src/skew_symmetric_network.o \
	${OBJECTDIR}/src/cycle_basis.o \
	${OBJECTDIR}/src/aux_path_finder.o \
	${OBJECTDIR}/src/graph_perfect_matching.o \
	${OBJECTDIR}/src/graph_affine_matcher.o \
	${OBJECTDIR}/src/scaffold_detection.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_sync_wrapper.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_c/nano_posix.o \
	${OBJECTDIR}/src/embedding_enumerator.o \
	${OBJECTDIR}/src/simple_cycle_basis.o \
	${OBJECTDIR}/src/automorphism_search.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/smart_output.o \
	${OBJECTDIR}/src/graph_subchain_enumerator.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/crc32.o \
	${OBJECTDIR}/src/graph_constrained_bmatching_finder.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/tlscont.o \
	${OBJECTDIR}/src/tree_canonizer.o \
	${OBJECTDIR}/src/path_enumerator.o \
	${OBJECTDIR}/src/filter.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/d_bitset.o \
	${OBJECTDIR}/src/edge_rotation_matcher.o \
	${OBJECTDIR}/src/graph.o \
	${OBJECTDIR}/src/graph_subtree_enumerator.o \
	${OBJECTDIR}/src/skew_symmetric_flow_finder.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/profiling.o \
	${OBJECTDIR}/src/dfs_walk.o \
	${OBJECTDIR}/src/biconnected_decomposer.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_tls_posix.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_sync_posix.o \
	${OBJECTDIR}/src/shortest_path_finder.o \
	${OBJECTDIR}/src/morgan_code.o \
	${OBJECTDIR}/src/spanning_tree.o \
	${OBJECTDIR}/src/graph_decomposer.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_thread_posix.o \
	${OBJECTDIR}/src/embeddings_storage.o \
	${OBJECTDIR}/src/subgraph_hash.o \
	${OBJECTDIR}/src/graph_highlighting.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_thread_wrapper.o \
	${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/exception.o \
	${OBJECTDIR}/src/max_common_subgraph.o \
	${OBJECTDIR}/src/ring_canonizer.o

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
	${MAKE}  -f nbproject/Makefile-ReleaseIntel64.mk dist/ReleaseIntel64/ICC-Linux-x86/libgraph.a

dist/ReleaseIntel64/ICC-Linux-x86/libgraph.a: ${OBJECTFILES}
	${MKDIR} -p dist/ReleaseIntel64/ICC-Linux-x86
	${RM} dist/ReleaseIntel64/ICC-Linux-x86/libgraph.a
	${AR} rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libgraph.a ${OBJECTFILES} 
	$(RANLIB) dist/ReleaseIntel64/ICC-Linux-x86/libgraph.a

${OBJECTDIR}/src/cycle_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/cycle_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cycle_enumerator.o src/cycle_enumerator.cpp

${OBJECTDIR}/src/skew_symmetric_network.o: nbproject/Makefile-${CND_CONF}.mk src/skew_symmetric_network.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/skew_symmetric_network.o src/skew_symmetric_network.cpp

${OBJECTDIR}/src/cycle_basis.o: nbproject/Makefile-${CND_CONF}.mk src/cycle_basis.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cycle_basis.o src/cycle_basis.cpp

${OBJECTDIR}/src/aux_path_finder.o: nbproject/Makefile-${CND_CONF}.mk src/aux_path_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/aux_path_finder.o src/aux_path_finder.cpp

${OBJECTDIR}/src/graph_perfect_matching.o: nbproject/Makefile-${CND_CONF}.mk src/graph_perfect_matching.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_perfect_matching.o src/graph_perfect_matching.cpp

${OBJECTDIR}/src/graph_affine_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/graph_affine_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_affine_matcher.o src/graph_affine_matcher.cpp

${OBJECTDIR}/src/scaffold_detection.o: nbproject/Makefile-${CND_CONF}.mk src/scaffold_detection.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/scaffold_detection.o src/scaffold_detection.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_sync_wrapper.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/os_sync_wrapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_sync_wrapper.o ../common/base_cpp/os_sync_wrapper.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_c/nano_posix.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_c/nano_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_c
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_c/nano_posix.o ../common/base_c/nano_posix.c

${OBJECTDIR}/src/embedding_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/embedding_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/embedding_enumerator.o src/embedding_enumerator.cpp

${OBJECTDIR}/src/simple_cycle_basis.o: nbproject/Makefile-${CND_CONF}.mk src/simple_cycle_basis.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/simple_cycle_basis.o src/simple_cycle_basis.cpp

${OBJECTDIR}/src/automorphism_search.o: nbproject/Makefile-${CND_CONF}.mk src/automorphism_search.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/automorphism_search.o src/automorphism_search.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/smart_output.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/smart_output.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/smart_output.o ../common/base_cpp/smart_output.cpp

${OBJECTDIR}/src/graph_subchain_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/graph_subchain_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_subchain_enumerator.o src/graph_subchain_enumerator.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/crc32.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/crc32.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/crc32.o ../common/base_cpp/crc32.cpp

${OBJECTDIR}/src/graph_constrained_bmatching_finder.o: nbproject/Makefile-${CND_CONF}.mk src/graph_constrained_bmatching_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_constrained_bmatching_finder.o src/graph_constrained_bmatching_finder.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/tlscont.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/tlscont.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/tlscont.o ../common/base_cpp/tlscont.cpp

${OBJECTDIR}/src/tree_canonizer.o: nbproject/Makefile-${CND_CONF}.mk src/tree_canonizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/tree_canonizer.o src/tree_canonizer.cpp

${OBJECTDIR}/src/path_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/path_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/path_enumerator.o src/path_enumerator.cpp

${OBJECTDIR}/src/filter.o: nbproject/Makefile-${CND_CONF}.mk src/filter.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/filter.o src/filter.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/d_bitset.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/d_bitset.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/d_bitset.o ../common/base_cpp/d_bitset.cpp

${OBJECTDIR}/src/edge_rotation_matcher.o: nbproject/Makefile-${CND_CONF}.mk src/edge_rotation_matcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/edge_rotation_matcher.o src/edge_rotation_matcher.cpp

${OBJECTDIR}/src/graph.o: nbproject/Makefile-${CND_CONF}.mk src/graph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph.o src/graph.cpp

${OBJECTDIR}/src/graph_subtree_enumerator.o: nbproject/Makefile-${CND_CONF}.mk src/graph_subtree_enumerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_subtree_enumerator.o src/graph_subtree_enumerator.cpp

${OBJECTDIR}/src/skew_symmetric_flow_finder.o: nbproject/Makefile-${CND_CONF}.mk src/skew_symmetric_flow_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/skew_symmetric_flow_finder.o src/skew_symmetric_flow_finder.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/profiling.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/profiling.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/profiling.o ../common/base_cpp/profiling.cpp

${OBJECTDIR}/src/dfs_walk.o: nbproject/Makefile-${CND_CONF}.mk src/dfs_walk.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/dfs_walk.o src/dfs_walk.cpp

${OBJECTDIR}/src/biconnected_decomposer.o: nbproject/Makefile-${CND_CONF}.mk src/biconnected_decomposer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/biconnected_decomposer.o src/biconnected_decomposer.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_tls_posix.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_c/os_tls_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_c
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_tls_posix.o ../common/base_c/os_tls_posix.c

${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_sync_posix.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_c/os_sync_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_c
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_sync_posix.o ../common/base_c/os_sync_posix.c

${OBJECTDIR}/src/shortest_path_finder.o: nbproject/Makefile-${CND_CONF}.mk src/shortest_path_finder.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/shortest_path_finder.o src/shortest_path_finder.cpp

${OBJECTDIR}/src/morgan_code.o: nbproject/Makefile-${CND_CONF}.mk src/morgan_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/morgan_code.o src/morgan_code.cpp

${OBJECTDIR}/src/spanning_tree.o: nbproject/Makefile-${CND_CONF}.mk src/spanning_tree.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/spanning_tree.o src/spanning_tree.cpp

${OBJECTDIR}/src/graph_decomposer.o: nbproject/Makefile-${CND_CONF}.mk src/graph_decomposer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_decomposer.o src/graph_decomposer.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_thread_posix.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_c/os_thread_posix.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_c
	${RM} $@.d
	$(COMPILE.c) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_c/os_thread_posix.o ../common/base_c/os_thread_posix.c

${OBJECTDIR}/src/embeddings_storage.o: nbproject/Makefile-${CND_CONF}.mk src/embeddings_storage.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/embeddings_storage.o src/embeddings_storage.cpp

${OBJECTDIR}/src/subgraph_hash.o: nbproject/Makefile-${CND_CONF}.mk src/subgraph_hash.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/subgraph_hash.o src/subgraph_hash.cpp

${OBJECTDIR}/src/graph_highlighting.o: nbproject/Makefile-${CND_CONF}.mk src/graph_highlighting.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/graph_highlighting.o src/graph_highlighting.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_thread_wrapper.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/os_thread_wrapper.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/os_thread_wrapper.o ../common/base_cpp/os_thread_wrapper.cpp

${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/exception.o: nbproject/Makefile-${CND_CONF}.mk ../common/base_cpp/exception.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/common/base_cpp/exception.o ../common/base_cpp/exception.cpp

${OBJECTDIR}/src/max_common_subgraph.o: nbproject/Makefile-${CND_CONF}.mk src/max_common_subgraph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/max_common_subgraph.o src/max_common_subgraph.cpp

${OBJECTDIR}/src/ring_canonizer.o: nbproject/Makefile-${CND_CONF}.mk src/ring_canonizer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I../common -I.. -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ring_canonizer.o src/ring_canonizer.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ReleaseIntel64
	${RM} dist/ReleaseIntel64/ICC-Linux-x86/libgraph.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
