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
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/molecule_layout_graph.o \
	${OBJECTDIR}/src/molecule_layout.o \
	${OBJECTDIR}/src/molecule_layout_graph_cycle.o \
	${OBJECTDIR}/src/molecule_layout_graph_geom.o \
	${OBJECTDIR}/src/molecule_layout_graph_border.o \
	${OBJECTDIR}/src/layout_pattern.o \
	${OBJECTDIR}/src/molecule_layout_graph_assign.o \
	${OBJECTDIR}/src/refinement_state.o \
	${OBJECTDIR}/src/molecule_layout_graph_refine.o \
	${OBJECTDIR}/src/attachment_layout.o \
	${OBJECTDIR}/src/molecule_layout_graph_attach.o \
	${OBJECTDIR}/src/metalayout.o \
	${OBJECTDIR}/src/reaction_layout.o


# C Compiler Flags
CFLAGS=-fPIC

# CC Compiler Flags
CCFLAGS=-fPIC
CXXFLAGS=-fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/liblayout.a

dist/Debug/GNU-Linux-x86/liblayout.a: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${RM} dist/Debug/GNU-Linux-x86/liblayout.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblayout.a ${OBJECTFILES} 
	$(RANLIB) dist/Debug/GNU-Linux-x86/liblayout.a

${OBJECTDIR}/src/molecule_layout_graph.o: src/molecule_layout_graph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph.o src/molecule_layout_graph.cpp

${OBJECTDIR}/src/molecule_layout.o: src/molecule_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout.o src/molecule_layout.cpp

${OBJECTDIR}/src/molecule_layout_graph_cycle.o: src/molecule_layout_graph_cycle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_cycle.o src/molecule_layout_graph_cycle.cpp

${OBJECTDIR}/src/molecule_layout_graph_geom.o: src/molecule_layout_graph_geom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_geom.o src/molecule_layout_graph_geom.cpp

${OBJECTDIR}/src/molecule_layout_graph_border.o: src/molecule_layout_graph_border.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_border.o src/molecule_layout_graph_border.cpp

${OBJECTDIR}/src/layout_pattern.o: src/layout_pattern.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/layout_pattern.o src/layout_pattern.cpp

${OBJECTDIR}/src/molecule_layout_graph_assign.o: src/molecule_layout_graph_assign.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_assign.o src/molecule_layout_graph_assign.cpp

${OBJECTDIR}/src/refinement_state.o: src/refinement_state.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/refinement_state.o src/refinement_state.cpp

${OBJECTDIR}/src/molecule_layout_graph_refine.o: src/molecule_layout_graph_refine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_refine.o src/molecule_layout_graph_refine.cpp

${OBJECTDIR}/src/attachment_layout.o: src/attachment_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/attachment_layout.o src/attachment_layout.cpp

${OBJECTDIR}/src/molecule_layout_graph_attach.o: src/molecule_layout_graph_attach.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_attach.o src/molecule_layout_graph_attach.cpp

${OBJECTDIR}/src/metalayout.o: src/metalayout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/metalayout.o src/metalayout.cpp

${OBJECTDIR}/src/reaction_layout.o: src/reaction_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_layout.o src/reaction_layout.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/liblayout.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
