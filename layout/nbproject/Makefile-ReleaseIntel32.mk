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
CND_CONF=ReleaseIntel32
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/molecule_layout_graph_cycle.o \
	${OBJECTDIR}/src/molecule_layout_graph_border.o \
	${OBJECTDIR}/src/molecule_layout_graph_assign.o \
	${OBJECTDIR}/src/molecule_layout_graph_refine.o \
	${OBJECTDIR}/src/refinement_state.o \
	${OBJECTDIR}/src/molecule_layout_graph.o \
	${OBJECTDIR}/src/molecule_layout_graph_attach.o \
	${OBJECTDIR}/src/attachment_layout.o \
	${OBJECTDIR}/src/layout_pattern.o \
	${OBJECTDIR}/src/molecule_layout.o \
	${OBJECTDIR}/src/reaction_layout.o \
	${OBJECTDIR}/src/metalayout.o \
	${OBJECTDIR}/src/molecule_layout_graph_geom.o

# C Compiler Flags
CFLAGS=-m32 -fPIC

# CC Compiler Flags
CCFLAGS=-m32 -fPIC
CXXFLAGS=-m32 -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ReleaseIntel32.mk dist/ReleaseIntel32/ICC-Linux-x86/liblayout.a

dist/ReleaseIntel32/ICC-Linux-x86/liblayout.a: ${OBJECTFILES}
	${MKDIR} -p dist/ReleaseIntel32/ICC-Linux-x86
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/liblayout.a
	${AR} rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblayout.a ${OBJECTFILES} 
	$(RANLIB) dist/ReleaseIntel32/ICC-Linux-x86/liblayout.a

${OBJECTDIR}/src/molecule_layout_graph_cycle.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_cycle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_cycle.o src/molecule_layout_graph_cycle.cpp

${OBJECTDIR}/src/molecule_layout_graph_border.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_border.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_border.o src/molecule_layout_graph_border.cpp

${OBJECTDIR}/src/molecule_layout_graph_assign.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_assign.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_assign.o src/molecule_layout_graph_assign.cpp

${OBJECTDIR}/src/molecule_layout_graph_refine.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_refine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_refine.o src/molecule_layout_graph_refine.cpp

${OBJECTDIR}/src/refinement_state.o: nbproject/Makefile-${CND_CONF}.mk src/refinement_state.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/refinement_state.o src/refinement_state.cpp

${OBJECTDIR}/src/molecule_layout_graph.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph.o src/molecule_layout_graph.cpp

${OBJECTDIR}/src/molecule_layout_graph_attach.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_attach.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_attach.o src/molecule_layout_graph_attach.cpp

${OBJECTDIR}/src/attachment_layout.o: nbproject/Makefile-${CND_CONF}.mk src/attachment_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/attachment_layout.o src/attachment_layout.cpp

${OBJECTDIR}/src/layout_pattern.o: nbproject/Makefile-${CND_CONF}.mk src/layout_pattern.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/layout_pattern.o src/layout_pattern.cpp

${OBJECTDIR}/src/molecule_layout.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout.o src/molecule_layout.cpp

${OBJECTDIR}/src/reaction_layout.o: nbproject/Makefile-${CND_CONF}.mk src/reaction_layout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/reaction_layout.o src/reaction_layout.cpp

${OBJECTDIR}/src/metalayout.o: nbproject/Makefile-${CND_CONF}.mk src/metalayout.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/metalayout.o src/metalayout.cpp

${OBJECTDIR}/src/molecule_layout_graph_geom.o: nbproject/Makefile-${CND_CONF}.mk src/molecule_layout_graph_geom.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -s -I.. -I../common -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/molecule_layout_graph_geom.o src/molecule_layout_graph_geom.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ReleaseIntel32
	${RM} dist/ReleaseIntel32/ICC-Linux-x86/liblayout.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
